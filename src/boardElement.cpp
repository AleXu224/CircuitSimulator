#include "boardElement.hpp"
#include "component.hpp"
#include "gestureDetector.hpp"
#include "element.hpp"
#include "msdfImage.hpp"
#include <GLFW/glfw3.h>

using namespace squi;

void BoardElement::Rotate(squi::Widget &widget, uint32_t rotation, const Component &component) {
	vec2 newTopLeft{};
	vec2 newTopRight{};
	vec2 newBottomRight{};
	vec2 newBottomLeft{};
	switch (rotation % 4) {
		case 0: {
			newTopLeft = {component.uvTopLeft.x, component.uvTopLeft.y};
			newTopRight = {component.uvBottomRight.x, component.uvTopLeft.y};
			newBottomRight = {component.uvBottomRight.x, component.uvBottomRight.y};
			newBottomLeft = {component.uvTopLeft.x, component.uvBottomRight.y};
			break;
		}
		case 1: {
			newTopLeft = {component.uvTopLeft.x, component.uvBottomRight.y};
			newTopRight = {component.uvTopLeft.x, component.uvTopLeft.y};
			newBottomRight = {component.uvBottomRight.x, component.uvTopLeft.y};
			newBottomLeft = {component.uvBottomRight.x, component.uvBottomRight.y};
			break;
		}
		case 2: {
			newTopLeft = {component.uvBottomRight.x, component.uvBottomRight.y};
			newTopRight = {component.uvTopLeft.x, component.uvBottomRight.y};
			newBottomRight = {component.uvTopLeft.x, component.uvTopLeft.y};
			newBottomLeft = {component.uvBottomRight.x, component.uvTopLeft.y};
			break;
		}
		case 3: {
			newTopLeft = {component.uvBottomRight.x, component.uvTopLeft.y};
			newTopRight = {component.uvBottomRight.x, component.uvBottomRight.y};
			newBottomRight = {component.uvTopLeft.x, component.uvBottomRight.y};
			newBottomLeft = {component.uvTopLeft.x, component.uvTopLeft.y};
			break;
		}
	}

	auto &image = widget.as<MsdfImage::Impl>();

	if (component.width != component.height) {
		if (rotation % 2 == 0) {
			image.state.width = static_cast<float>(component.width) * 20.f;
			image.state.height = static_cast<float>(component.height) * 20.f;
		} else {
			image.state.width = static_cast<float>(component.height) * 20.f;
			image.state.height = static_cast<float>(component.width) * 20.f;
		}
	}

	image.setUv(newTopLeft, newTopRight, newBottomRight, newBottomLeft);
}

BoardElement::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.component = component,
	});

	return GestureDetector{
		.onFocus = [storage](GestureDetector::Event /*event*/) {
			storage->dragStartPos = GestureDetector::getMousePos();
		},
		.onInactive = [storage](GestureDetector::Event event) {
			event.widget.as<MsdfImage::Impl>().setColor(0xFFFFFFFF);
			storage->active = false;
		},
		.onClick = [storage](GestureDetector::Event event) {
			if ((storage->dragStartPos - GestureDetector::getMousePos()).length() > 5.f) return;
			event.widget.as<MsdfImage::Impl>().setColor(0xAAAAFFFF);
			storage->active = true;
		},
		.onUpdate = [storage](GestureDetector::Event event) {
			if (storage->active) {
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_DELETE)) event.widget.deleteLater();
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) event.state.setInactive();
			}
		},
		.child = MsdfImage{
			.widget{
				.width = static_cast<float>(component.width) * 20.f,
				.height = static_cast<float>(component.height) * 20.f,
				.customState{
					Element{
						.size{
							.x = static_cast<int32_t>(component.width),
							.y = static_cast<int32_t>(component.height),
						},
						.pos{position},
						.rotation = rotation,
					},
				},
				.onInit = [storage = storage](Widget &w){
					auto &element = w.customState.get<Element>();
					BoardElement::Rotate(w, element.rotation, storage->component);
				},
				.onArrange = [](Widget & w, auto &pos) {
					auto &elemPos = w.customState.get<Element>().pos;

					pos += vec2(static_cast<float>(elemPos.x), static_cast<float>(elemPos.y)) * 20.f;
				},
			},
			.texture = component.texture,
			.color{1.f, 1.f, 1.f, 0.5f},
			.uvTopLeft{component.uvTopLeft},
			.uvBottomRight{component.uvBottomRight},
		},
	};
}
