#include "boardview.hpp"
#include "boardElement.hpp"
#include "boardLine.hpp"
#include "boardSelection.hpp"
#include "column.hpp"
#include "compiledShaders/boardBackgroundfrag.hpp"
#include "compiledShaders/boardBackgroundvert.hpp"
#include "component.hpp"
#include "components/componentStore.hpp"
#include "elementState.hpp"
#include "gestureDetector.hpp"
#include "image.hpp"
#include "samplerUniform.hpp"
#include "topBar.hpp"
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
				  auto &w = selectedLineWidget.value();
				  bool continuePlacing = true;
				  if (auto it = boardStorage.nodes.find(coordsToGridRounded(GestureDetector::getMousePos())); it != boardStorage.nodes.end()) {
					  if (!it->second.second.empty()) continuePlacing = false;
				  }
				  w->customState.get<StateObservable>()->notify(ElementState::placed);
				  if (!continuePlacing) {
					selectedLineWidget.reset();
					return;
				  }
				  Child child = BoardLine{
					  .boardStorage = boardStorage,
					  .startPos = w->customState.get<BoardLine::Storage>().endPos,
				  };
				  addChild(child);
				  child->customState.get<StateObservable>()->notify(ElementState::placing);
				  selectedLineWidget = child;
			  } else if (selectedComponent.has_value() && selectedComponentWidget.has_value()) {
				  auto &componentWidget = selectedComponentWidget.value();
				  auto &elem = componentWidget->customState.get<Element>();
				  if (boardStorage.isElementOverlapping(elem)) return;
				  componentWidget->customState.get<StateObservable>()->notify(ElementState::placed);

				  auto &comp = selectedComponent.value();

				  squi::Child child = BoardElement{
					  .rotation = elem.rotation,
					  .component = comp.get(),
					  .boardStorage = boardStorage,
				  };
				  addChild(child);
				  child->customState.get<StateObservable>()->notify(ElementState::placing);
				  selectedComponent = comp.get();
				  selectedComponentWidget = child;
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
	  observer(componentSelectorObservable->observe([&](const std::reference_wrapper<const Component> &comp) {
		  if (selectedComponentWidget.has_value()) {
			  selectedComponentWidget.value()->deleteLater();
			  selectedComponentWidget.reset();
		  }

		  squi::Child child = BoardElement{
			  .component = comp.get(),
			  .boardStorage = boardStorage,
		  };
		  addChild(child);
		  child->customState.get<StateObservable>()->notify(ElementState::placing);
		  selectedComponent = comp.get();
		  selectedComponentWidget = child;
	  })) {
	funcs().onChildAdded.emplace_back([](Widget &, const Child &) {});
	funcs().onChildRemoved.emplace_back([&](Widget &, const Child &child) {
		child->customState.get<StateObservable>()->notify(ElementState::removed);
	});
}

void BoardView::Impl::onUpdate() {
	if (!loadedComponents) {
		for (const auto &comp: ComponentStore::components) {
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

	if (selectionWidget.has_value()) {
		if (GestureDetector::isKey(GLFW_MOUSE_BUTTON_1, GLFW_RELEASE, GLFW_MOD_CONTROL) ||
			GestureDetector::isKey(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE) ||
			GestureDetector::isKey(GLFW_KEY_RIGHT_CONTROL, GLFW_RELEASE)) {

			auto &storage = selectionWidget.value()->customState.get<BoardSelection::Storage>();
			Coords minPos{
				std::min(storage.startPos->x, storage.endPos->x),
				std::min(storage.startPos->y, storage.endPos->y),
			};
			Coords maxPos{
				std::max(storage.startPos->x, storage.endPos->x),
				std::max(storage.startPos->y, storage.endPos->y),
			};
			for (auto x = minPos.x; x < maxPos.x; x++) {
				for (auto y = minPos.y; y < maxPos.y; y++) {
					if (auto it = boardStorage.board.find(Coords{x, y}); it != boardStorage.board.end()) {
						if (it->second.expired()) continue;
						selectedWidgets.emplace_back(it->second);
						it->second.lock()->customState.get<StateObservable>()->notify(ElementState::selected);
					}
					if (auto it = boardStorage.lines.find(Coords{x, y}); it != boardStorage.lines.end()) {
						for (auto &line: it->second) {
							if (line.expired()) continue;
							selectedWidgets.emplace_back(line);
							line.lock()->customState.get<StateObservable>()->notify(ElementState::selected);
						}
					}
				}
			}

			selectionWidget.value()->deleteLater();
			selectionWidget.reset();
		} else {
			selectionWidget.value()->customState.get<BoardSelection::Storage>().endPos = coordsToGridRounded(GestureDetector::getMousePos());
		}
	}

	if (GestureDetector::canClick(*this)) {
		if (GestureDetector::isKey(GLFW_MOUSE_BUTTON_1, GLFW_PRESS, GLFW_MOD_CONTROL)) {
			selectionWidget = BoardSelection{
				.startPos = coordsToGridRounded(GestureDetector::getMousePos()),
			};
			addChild(selectionWidget.value());
		}
	}

	if (gd.focused) {
		if (GestureDetector::getMouseDelta().length() != 0.f) {
			viewOffset += GestureDetector::getMouseDelta();
			quad.offset = viewOffset;
			this->reArrange();
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE) || GestureDetector::isKeyPressedOrRepeat(GLFW_MOUSE_BUTTON_2)) {
		selectedComponent.reset();
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

	if (selectedComponentWidget.has_value()) {
		auto &element = selectedComponentWidget.value()->customState.get<Element>();

		if (selectedComponent.has_value()) {
			if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_R)) {
				element.rotation++;
				BoardElement::Rotate(*selectedComponentWidget.value(), element.rotation, selectedComponent.value().get());
			}
			if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_R, GLFW_MOD_SHIFT)) {
				element.rotation--;
				BoardElement::Rotate(*selectedComponentWidget.value(), element.rotation, selectedComponent.value().get());
			}
		}
		const auto mousePos = GestureDetector::getMousePos();
		const auto widgetPos = getPos();
		const auto virtualPos = mousePos - widgetPos - viewOffset;

		auto gridPos = virtualPos / gridWidth;
		gridPos.x = std::round(gridPos.x - (static_cast<float>(element.size.x) / 2.f));
		gridPos.y = std::round(gridPos.y - (static_cast<float>(element.size.y) / 2.f));
		auto newCoords = Coords{
			.x = static_cast<int32_t>(gridPos.x),
			.y = static_cast<int32_t>(gridPos.y),
		};
		auto &elem = selectedComponentWidget.value()->customState.get<Element>();
		if (elem.pos != newCoords) {
			elem.pos = newCoords;
			this->reArrange();
		}
	}

	if (selectedLineWidget.has_value()) {
		auto &storage = selectedLineWidget.value()->customState.get<BoardLine::Storage>();

		auto gridPos = coordsToGridRounded(GestureDetector::getMousePos());
		if (std::abs(storage.startPos->x - gridPos.x) < std::abs(storage.startPos->y - gridPos.y)) {
			storage.endPos = Coords{
				.x = storage.startPos->x,
				.y = gridPos.y,
			};
		} else {
			storage.endPos = Coords{
				.x = gridPos.x,
				.y = storage.startPos->y,
			};
		}
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
	for (auto &nodeIt: boardStorage.nodes) {
		auto &widget = nodeIt.second.first;
		if (nodeIt.second.second.empty()) continue;
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
	for (auto &nodeIt: boardStorage.nodes) {
		auto &widget = nodeIt.second.first;
		if (nodeIt.second.second.empty()) continue;
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
	for (auto &nodeIt: boardStorage.nodes) {
		auto &widget = nodeIt.second.first;
		if (nodeIt.second.second.empty()) continue;
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
	for (auto &nodeIt: boardStorage.nodes) {
		auto &widget = nodeIt.second.first;
		if (nodeIt.second.second.empty()) continue;
		if (!widget) continue;

		widget->draw();
	}
	instance.popScissor();
}

void BoardView::Impl::clickElement(squi::GestureDetector::Event /*event*/) {
	if (
		auto itNode = boardStorage.nodes.find(coordsToGridRounded(GestureDetector::getMousePos()));
		itNode != boardStorage.nodes.end() && !itNode->second.second.empty()
	) {
		selectedLineWidget = BoardLine{
			.boardStorage = boardStorage,
			.startPos = coordsToGridRounded(GestureDetector::getMousePos()),
		};
		addChild(selectedLineWidget.value());
		selectedLineWidget.value()->customState.get<StateObservable>()->notify(ElementState::placing);
	} else if (
		auto itLine = boardStorage.lines.find(coordsToGridRounded(GestureDetector::getMousePos()));
		itLine != boardStorage.lines.end() && !itLine->second.empty()
	) {
		for (auto &widget: itLine->second) {
			if (widget.expired()) continue;
			widget.lock()->customState.get<StateObservable>()->notify(ElementState::selected);
			selectedWidgets.emplace_back(widget);
		}
	} else if (
		auto itBoard = boardStorage.board.find(coordsToGridFloored(GestureDetector::getMousePos()));
		itBoard != boardStorage.board.end()
	) {
		if (itBoard->second.expired()) return;
		auto widget = itBoard->second.lock();
		widget->customState.get<StateObservable>()->notify(ElementState::selected);
		// FIXME: widgets can be added multiple times
		selectedWidgets.emplace_back(widget);
	} else {
		unselectAll();
	}
}

Coords BoardView::Impl::coordsToGridRounded(const squi::vec2 &input) const {
	const auto widgetPos = getPos();
	const auto virtualPos = input - widgetPos - viewOffset;

	auto gridPos = virtualPos / gridWidth;
	gridPos.x = std::round(gridPos.x);
	gridPos.y = std::round(gridPos.y);
	return Coords{
		.x = static_cast<int32_t>(gridPos.x),
		.y = static_cast<int32_t>(gridPos.y),
	};
}

Coords BoardView::Impl::coordsToGridFloored(const squi::vec2 &input) const {
	const auto widgetPos = getPos();
	const auto virtualPos = input - widgetPos - viewOffset;

	auto gridPos = virtualPos / gridWidth;
	gridPos.x = std::floor(gridPos.x);
	gridPos.y = std::floor(gridPos.y);
	return Coords{
		.x = static_cast<int32_t>(gridPos.x),
		.y = static_cast<int32_t>(gridPos.y),
	};
}

void BoardView::Impl::unselectAll() {
	for (auto &w: selectedWidgets) {
		if (w.expired()) continue;
		auto widget = w.lock();
		widget->customState.get<StateObservable>()->notify(ElementState::unselected);
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
			},
			ret,
		},
	};
}
