#include "boardview.hpp"
#include "boardElement.hpp"
#include "boardElementPlacer.hpp"
#include "boardLine.hpp"
#include "boardLinePlacer.hpp"
#include "boardSelection.hpp"
#include "column.hpp"
#include "compiledShaders/boardBackgroundfrag.hpp"
#include "compiledShaders/boardBackgroundvert.hpp"
#include "component.hpp"
#include "components/componentStore.hpp"
#include "elementState.hpp"
#include "gestureDetector.hpp"
#include "image.hpp"
#include "propertyEditor.hpp"
#include "samplerUniform.hpp"
#include "topBar.hpp"
#include "utils.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <print>


using namespace squi;

BoardView::Pipeline *BoardView::pipeline = nullptr;

BoardView::Impl::Impl(const BoardView &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  gd(GestureDetector{
		  .onClick = [&](GestureDetector::Event event) {
			  if ((GestureDetector::getMousePos() - event.state.getDragStartPos()).length() > 5.f) return;

			  if (selectedLineWidget.has_value()) {
				  const auto clickCoords = Utils::screenToGridRounded(GestureDetector::getMousePos(), viewOffset, getPos());
				  bool shouldStop = !boardStorage.connections[clickCoords].connections.empty();
				  selectedLineWidget.value()->customState.get<VoidObserver>().notifyOthers();
				  if (shouldStop) {
					  selectedLineWidget.value()->deleteLater();
					  selectedLineWidget.reset();
				  }
			  } else if (selectedComponentWidget.has_value()) {
				  selectedComponentWidget.value()->customState.get<VoidObserver>().notifyOthers();
			  } else {
				  clickElement(event);
			  }
		  },
	  }
			 .mount(*this)),
	  quad(Quad::Args{
		  .position{0, 0},
		  .size{0, 0},
		  .offset{0, 0},
	  }),
	  observer(componentSelectorObservable.observe([&](const std::reference_wrapper<const Component> &comp) {
		  if (selectedComponentWidget.has_value()) {
			  selectedComponentWidget.value()->deleteLater();
			  selectedComponentWidget.reset();
		  }

		  Child child = BoardElementPlacer{
			  .boardStorage = boardStorage,
			  .component = comp.get(),
			  .viewOffset = viewOffset,
			  .board = weak_from_this(),
		  };

		  addChild(child);
		  selectedComponentWidget = child;
	  })) {}

void BoardView::Impl::onUpdate() {
	// Initialize component textures
	if (!loadedComponents) {
		uint32_t idCounter = 0;
		for (const auto &comp: ComponentStore::components) {
			const_cast<uint32_t &>(comp.get().id) = idCounter++;
			auto job = std::thread([&comp, &instance = Window::of(this).engine.instance] {
				{
					auto data = Image::Data::fromFile(comp.get().texturePath);

					auto &sampler = const_cast<std::optional<Engine::SamplerUniform> &>(comp.get().texture).emplace(Engine::SamplerUniform::Args{
						.instance = instance,
						.textureArgs{
							.instance = instance,
							.width = static_cast<uint32_t>(data.width),
							.height = static_cast<uint32_t>(data.height),
							.channels = static_cast<uint32_t>(data.channels),
						},
					});

					auto layout = sampler.texture.image.getSubresourceLayout(vk::ImageSubresource{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = 0,
						.arrayLayer = 0,
					});
					for (int row = 0; row < data.height; row++) {
						memcpy(
							reinterpret_cast<uint8_t *>(sampler.texture.mappedMemory) + row * layout.rowPitch,
							data.data.data() + static_cast<ptrdiff_t>(row * data.width * data.channels),
							static_cast<size_t>(data.width) * data.channels
						);
					}
				}
				{
					auto data = Image::Data::fromFile(comp.get().textureThumbPath);

					auto &sampler = const_cast<std::optional<Engine::SamplerUniform> &>(comp.get().textureThumb).emplace(Engine::SamplerUniform::Args{
						.instance = instance,
						.textureArgs{
							.instance = instance,
							.width = static_cast<uint32_t>(data.width),
							.height = static_cast<uint32_t>(data.height),
							.channels = static_cast<uint32_t>(data.channels),
						},
					});

					auto layout = sampler.texture.image.getSubresourceLayout(vk::ImageSubresource{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = 0,
						.arrayLayer = 0,
					});
					for (int row = 0; row < data.height; row++) {
						memcpy(
							reinterpret_cast<uint8_t *>(sampler.texture.mappedMemory) + row * layout.rowPitch,
							data.data.data() + static_cast<ptrdiff_t>(row * data.width * data.channels),
							static_cast<size_t>(data.width) * data.channels
						);
					}
				}
			});
			job.detach();
		}
		loadedComponents = true;
	}

	const auto roundedGridPos = Utils::screenToGridRounded(GestureDetector::getMousePos(), viewOffset, getPos());

	// Update drag selection
	if (selectionWidget.has_value()) {
		if (GestureDetector::isKey(GLFW_MOUSE_BUTTON_1, GLFW_RELEASE, GLFW_MOD_CONTROL) ||
			GestureDetector::isKey(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE) ||
			GestureDetector::isKey(GLFW_KEY_RIGHT_CONTROL, GLFW_RELEASE)) {

			auto &storage = selectionWidget.value()->customState.get<BoardSelection::Storage>();
			auto minPos = Coords::min(storage.startPos, storage.endPos);
			auto maxPos = Coords::max(storage.startPos, storage.endPos);
			for (auto x: std::views::iota(minPos.x, maxPos.x)) {
				for (auto y: std::views::iota(minPos.y, maxPos.y)) {
					if (auto it = boardStorage.elementTiles.find(Coords{x, y}); it != boardStorage.elementTiles.end()) {
						selectedWidgets.emplace_back(it->second);
						auto elem = boardStorage.getElement(it->second);
						elem->get().widget->customState.get<StateObservable>().notify(ElementState::selected);
					}
					if (auto it = boardStorage.lineTiles.find(Coords{x, y}); it != boardStorage.lineTiles.end()) {
						for (auto &line: it->second) {
							selectedWidgets.emplace_back(line);
							auto elem = boardStorage.getLine(line);
							elem->get().widget->customState.get<StateObservable>().notify(ElementState::selected);
						}
					}
				}
			}

			selectionWidget.value()->deleteLater();
			selectionWidget.reset();
		} else {
			selectionWidget.value()->customState.get<BoardSelection::Storage>().endPos = roundedGridPos;
		}
	}

	// Initialize drag selection
	if (GestureDetector::canClick(*this)) {
		if (GestureDetector::isKey(GLFW_MOUSE_BUTTON_1, GLFW_PRESS, GLFW_MOD_CONTROL)) {
			selectionWidget = BoardSelection{
				.startPos = roundedGridPos,
			};
			addChild(selectionWidget.value());
		}
	}

	// Drag view
	if (gd.focused) {
		if (GestureDetector::getMouseDelta().length() != 0.f) {
			viewOffset += GestureDetector::getMouseDelta();
			quad.offset = viewOffset;
			this->reArrange();
		}
	}

	if (GestureDetector::isKeyPressedOrRepeat(GLFW_MOUSE_BUTTON_2) && !selectedComponentWidget.has_value() && !selectedLineWidget.has_value()) {
		if (auto it = boardStorage.elementTiles.find(Utils::screenToGridFloored(GestureDetector::getMousePos(), viewOffset, getPos())); it != boardStorage.elementTiles.end()) {
			auto elemData = boardStorage.getElement(it->second);
			Window::of(this).addOverlay(PropertyEditor{.element = elemData->get().element});
		}
	}

	// Escape or Mouse2 -> unselect all selected elements
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE) || GestureDetector::isKeyPressedOrRepeat(GLFW_MOUSE_BUTTON_2)) {
		if (selectedComponentWidget.has_value()) {
			selectedComponentWidget.value()->deleteLater();
			selectedComponentWidget.reset();
		}
		if (selectedLineWidget.has_value()) {
			selectedLineWidget.value()->deleteLater();
			selectedLineWidget.reset();
		}
		unselectAll();
	}

	// Delete or Backspace -> remove all selected elements
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_DELETE) || GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
		deleteSelected();
	}
}

void BoardView::Impl::postArrange(squi::vec2 &pos) {
	quad.position = pos;
}

void BoardView::Impl::postLayout(squi::vec2 &size) {
	quad.size = size;
}

void BoardView::Impl::onDraw() {
	if (!BoardView::pipeline) {
		BoardView::pipeline = &Window::of(this).engine.instance.createPipeline<BoardView::Pipeline>(BoardView::Pipeline::Args{
			.vertexShader = Engine::Shaders::boardBackgroundvert,
			.fragmentShader = Engine::Shaders::boardBackgroundfrag,
			.instance = Window::of(this).engine.instance,
		});
	}

	pipeline->bind();
	auto [vi, ii] = pipeline->getIndexes();
	pipeline->addData(quad.getData(vi, ii));
}

void BoardView::Impl::updateChildren() {
	Widget::updateChildren();
	const auto _ = {boardStorage.lines, boardStorage.elements};
	for (const auto &line: _ | std::views::join) {
		const auto &widget = line.widget;
		if (!widget) continue;
		widget->state.parent = this;
		widget->state.root = state.root;
		widget->update();
	}
	for (auto &[key, val]: boardStorage.connections) {
		auto &widget = val.widget;
		if (val.connections.empty()) continue;
		if (!widget) continue;

		widget->state.parent = this;
		widget->state.root = state.root;
		widget->update();
	}
}

squi::vec2 BoardView::Impl::layoutChildren(squi::vec2 /*maxSize*/, squi::vec2 /*minSize*/, ShouldShrink /*shouldShrink*/) {
	for (auto &child: getChildren()) {
		if (!child) continue;
		child->layout(vec2::infinity(), vec2{}, {false, false});
	}
	const auto _ = {boardStorage.lines, boardStorage.elements};
	for (const auto &line: _ | std::views::join) {
		const auto &widget = line.widget;
		if (!widget) continue;
		widget->layout(vec2::infinity(), {});
	}
	for (auto &[key, val]: boardStorage.connections) {
		auto &widget = val.widget;
		if (val.connections.empty()) continue;
		if (!widget) continue;

		widget->layout(vec2::infinity(), {}, {false, false});
	}
	return {};
}

void BoardView::Impl::arrangeChildren(squi::vec2 &pos) {
	const auto newPos = pos + viewOffset;
	for (auto &child: getChildren()) {
		if (!child) continue;
		child->arrange(newPos);
	}
	const auto _ = {boardStorage.lines, boardStorage.elements};
	for (const auto &line: _ | std::views::join) {
		const auto &widget = line.widget;
		if (!widget) continue;
		widget->arrange(newPos);
	}
	for (auto &[key, val]: boardStorage.connections) {
		auto &widget = val.widget;
		if (val.connections.empty()) continue;
		if (!widget) continue;

		widget->arrange(newPos);
	}
}

void BoardView::Impl::drawChildren() {
	auto &instance = Window::of(this).engine.instance;
	instance.pushScissor(getRect());
	for (auto &child: getChildren()) {
		if (!child) continue;
		child->draw();
	}
	const auto _ = {boardStorage.lines, boardStorage.elements};
	for (const auto &line: _ | std::views::join) {
		const auto &widget = line.widget;
		if (!widget) continue;
		widget->draw();
	}
	for (auto &[key, val]: boardStorage.connections) {
		auto &widget = val.widget;
		if (val.connections.empty()) continue;
		if (val.connections.size() == 2) continue;
		if (!widget) continue;

		widget->draw();
	}
	instance.popScissor();
}

void BoardView::Impl::clickElement(squi::GestureDetector::Event /*event*/) {
	const auto roundedGridPos = Utils::screenToGridRounded(GestureDetector::getMousePos(), viewOffset, getPos());

	if (
		auto itNode = boardStorage.connections.find(roundedGridPos);
		itNode != boardStorage.connections.end() && !itNode->second.connections.empty()
	) {
		selectedLineWidget = BoardLinePlacer{
			.startPos = roundedGridPos,
			.boardStorage = boardStorage,
			.component = ComponentStore::components.at(0),
			.viewOffset = viewOffset,
			.board = weak_from_this(),
		};
		addChild(selectedLineWidget.value());
	} else if (
		auto itLine = boardStorage.lineTiles.find(roundedGridPos);
		itLine != boardStorage.lineTiles.end() && !itLine->second.empty()
	) {
		for (auto &id: itLine->second) {
			auto line = boardStorage.getLine(id);
			const auto &widget = line->get().widget;
			widget->customState.get<StateObservable>().notify(ElementState::selected);
			selectedWidgets.emplace_back(id);
		}
	} else if (
		auto itBoard = boardStorage.elementTiles.find(Utils::screenToGridFloored(GestureDetector::getMousePos(), viewOffset, getPos()));
		itBoard != boardStorage.elementTiles.end()
	) {
		auto elem = boardStorage.getElement(itBoard->second);
		auto widget = elem->get().widget;
		widget->customState.get<StateObservable>().notify(ElementState::selected);
		// FIXME: widgets can be added multiple times
		selectedWidgets.emplace_back(itBoard->second);
	} else {
		unselectAll();
	}
}

void BoardView::Impl::unselectAll() {
	for (auto &id: selectedWidgets) {
		auto elem = boardStorage.getElement(id);
		auto line = boardStorage.getLine(id);
		if (!elem.has_value() && !line.has_value()) continue;
		auto widget = std::invoke([&]() {
			if (elem.has_value()) return elem->get().widget;
			return line->get().widget;
		});
		widget->customState.get<StateObservable>().notify(ElementState::unselected);
	}
	selectedWidgets.clear();
}

void BoardView::Impl::deleteSelected() {
	for (auto &id: selectedWidgets) {
		auto elem = boardStorage.getElement(id);
		auto line = boardStorage.getLine(id);
		if (!elem.has_value() && !line.has_value()) continue;
		auto widget = std::invoke([&]() {
			if (elem.has_value()) return elem->get().widget;
			return line->get().widget;
		});
		widget->customState.get<StateObservable>().notify(ElementState::removed);
	}
	selectedWidgets.clear();
}

BoardView::Impl::~Impl() {
	for (const auto &comp: ComponentStore::components) {
		const_cast<std::optional<Engine::SamplerUniform> &>(comp.get().texture).reset();
		const_cast<std::optional<Engine::SamplerUniform> &>(comp.get().textureThumb).reset();
	}
}

BoardView::operator squi::Child() const {
	auto ret = std::make_shared<Impl>(*this);
	return Column{
		.children{
			TopBar{
				.componentSelectorObserver = ret->componentSelectorObservable,
				.boardStorage = ret->boardStorage,
			},
			ret,
		},
	};
}
