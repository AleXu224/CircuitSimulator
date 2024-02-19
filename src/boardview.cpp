#include "boardview.hpp"
#include "boardElement.hpp"
#include "column.hpp"
#include "compiledShaders/boardBackgroundfrag.hpp"
#include "compiledShaders/boardBackgroundvert.hpp"
#include "component.hpp"
#include "components/componentStore.hpp"
#include "gestureDetector.hpp"
#include "image.hpp"
#include "msdfImage.hpp"
#include "samplerUniform.hpp"
#include "topBar.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <optional>
#include <print>

using namespace squi;

BoardView::Pipeline *BoardView::pipeline = nullptr;

BoardView::Impl::Impl(const BoardView &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  gd(GestureDetector{
		  .onClick = [&](GestureDetector::Event event) {
			  if (!selectedComponent.has_value()) return;
			  if (!selectedComponentWidget.has_value()) return;
			  if ((GestureDetector::getMousePos() - event.state.getDragStartPos()).length() > 5.f) return;
			  auto &componentWidget = selectedComponentWidget.value();
			  auto &elem = componentWidget->customState.get<Element>();
			  if (boardStorage.isElementOverlapping(elem)) return;
			  componentWidget->flags.isInteractive = true;
			  componentWidget->as<MsdfImage::Impl>().setColor(0xFFFFFFFF);

			  boardStorage.placeElement(componentWidget->weak_from_this());

			  auto &comp = selectedComponent.value();

			  squi::Child child = BoardElement{
				  .rotation = elem.rotation,
				  .component = comp.get(),
			  };
			  child->flags.isInteractive = false;
			  addChild(child);
			  selectedComponent = comp.get();
			  selectedComponentWidget = child;
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
		  };
		  child->flags.isInteractive = false;
		  addChild(child);
		  selectedComponent = comp.get();
		  selectedComponentWidget = child;
	  })) {
	funcs().onChildAdded.emplace_back([](Widget &, const Child &) {});
	funcs().onChildRemoved.emplace_back([&](Widget &, const Child &child) {
		auto &elem = child->customState.get<Element>();
		boardStorage.removeElement(elem);
	});
}

void BoardView::Impl::onUpdate() {
	if (!loadedComponents) {
		for (const auto &comp: ComponentStore::components) {
			auto job = std::thread([&comp, &instance = Window::of(this).engine.instance] {
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
			});
			job.detach();
		}
		loadedComponents = true;
	}

	static bool first = true;
	if (gd.focused) {
		if (!first) {
			first = true;
		} else if (GestureDetector::getMouseDelta().length() != 0.f) {
			viewOffset += GestureDetector::getMouseDelta();
			quad.offset = viewOffset;
			this->reArrange();
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
		selectedComponent.reset();
		if (selectedComponentWidget.has_value()) {
			selectedComponentWidget.value()->deleteLater();
			selectedComponentWidget.reset();
		}
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
		auto elemSize = element.getSize();
		gridPos.x = std::round(gridPos.x - (static_cast<float>(elemSize.x) / 2.f));
		gridPos.y = std::round(gridPos.y - (static_cast<float>(elemSize.y) / 2.f));
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
}

squi::vec2 BoardView::Impl::layoutChildren(squi::vec2 /*maxSize*/, squi::vec2 /*minSize*/, ShouldShrink /*shouldShrink*/) {
	for (auto &child: getChildren()) {
		if (!child) continue;
		child->layout(vec2::infinity(), vec2{}, {false, false});
	}
	return {};
}

void BoardView::Impl::arrangeChildren(squi::vec2 &pos) {
	const auto newPos = pos + viewOffset;
	for (auto &child: getChildren()) {
		if (!child) continue;
		child->arrange(newPos);
	}
}

void BoardView::Impl::drawChildren() {
	auto &instance = Window::of(this).engine.instance;
	instance.pushScissor(getRect());
	for (auto &child: getChildren()) {
		if (!child) continue;
		child->draw();
	}
	instance.popScissor();
}

BoardView::Impl::~Impl() {
	for (const auto &comp: ComponentStore::components) {
		const_cast<std::optional<Engine::SamplerUniform> &>(comp.get().texture).reset();
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
