#include "boardElement.hpp"
#include "boardStorage.hpp"
#include "component.hpp"
#include "config.hpp"
#include "coords.hpp"
#include "element.hpp"
#include "elementState.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "stack.hpp"
#include "stateContainer.hpp"
#include "text.hpp"
#include <GLFW/glfw3.h>


using namespace squi;

BoardElement::operator squi::Child() const {
	auto obs = StateObservable{};
	auto storage = std::make_shared<Storage>(Storage{
		.selected = false,
		.boardStorage = boardStorage,
		.elementId = element.id,
	});

	return GestureDetector{
		.onUpdate = [storage](GestureDetector::Event) {
			if (storage->selected) {
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_DELETE) || GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
					storage->boardStorage.removeElement(storage->elementId);
				}
			}
		},
		.child = Stack{
			.widget{
				.width = static_cast<float>(element.size.x) * gridSize,
				.height = static_cast<float>(element.size.y) * gridSize,
				.customState{obs},
			},
			.children{
				MsdfImage{
					.widget{
						.onInit = [rotationData = Utils::rotateElement(element.rotation, element.component), obs, storage](Widget &w) {
							auto &image = w.as<MsdfImage::Impl>();
							image.setUv(rotationData.newTopLeft, rotationData.newTopRight, rotationData.newBottomRight, rotationData.newBottomLeft);

							w.customState.add(
								obs.observe(
									[widget = w.weak_from_this(), storage](const ElementState &state) {
										if (widget.expired()) return;
										auto &img = widget.lock()->as<MsdfImage::Impl>();
										switch (state) {
											case ElementState::selected: {
												img.setColor(0xAAAAFFFF);
												storage->selected = true;
												return;
											}
											case ElementState::unselected: {
												storage->selected = false;
												img.setColor(0xFFFFFFFF);
												return;
											}
											case ElementState::removed: {
												storage->boardStorage.removeElement(storage->elementId);
												return;
											}
										}
									}
								)
							);
						},
						.onArrange = [storage](Widget &, auto &pos) {
							const auto &elem = storage->boardStorage.getElement(storage->elementId)->get();

							pos += vec2(static_cast<float>(elem.element.pos.x), static_cast<float>(elem.element.pos.y)) * gridSize;
						},
					},
					.texture = element.component.get().texture,
					.color{1.f, 1.f, 1.f, 1.f},
					.uvTopLeft{element.component.get().uvTopLeft},
					.uvBottomRight{element.component.get().uvBottomRight},
				},
				// Element name Text
				Text{
					.widget{
						.onArrange = [storage](Widget &w, vec2 &pos) {
							const auto &elem = storage->boardStorage.getElement(storage->elementId)->get();
							pos += vec2(static_cast<float>(elem.element.pos.x), static_cast<float>(elem.element.pos.y)) * gridSize;

							if (elem.element.rotation % 2 == 1) {
								pos.x += static_cast<float>(elem.element.size.x) * gridSize * 0.5f - w.getSize().x / 2.f;
								pos.y -= w.getSize().y;
							} else {
								pos.x += static_cast<float>(elem.element.size.x) * gridSize + 2.f;
								pos.y += static_cast<float>(elem.element.size.y) * gridSize * 0.5f - w.getSize().y;
							}
						},
					},
					.text = std::format("{}{}", element.component.get().prefix, element.id),
					.fontSize = 14.f,
				},
				// Value text
				Text{
					.widget{
						.onUpdate = [storage](Widget &w) {
							const auto &elem = storage->boardStorage.getElement(storage->elementId)->get();
							auto &text = w.as<Text::Impl>();
							if (storage->lastValue != elem.element.propertiesValues.front()) {
								text.setText(std::format("{}{}", elem.element.propertiesValues.front(), elem.element.component.get().properties.front().suffix));
							}
						},
						.onArrange = [storage](Widget &w, vec2 &pos) {
							const auto &elem = storage->boardStorage.getElement(storage->elementId)->get();
							pos += vec2(static_cast<float>(elem.element.pos.x), static_cast<float>(elem.element.pos.y)) * gridSize;

							if (elem.element.rotation % 2 == 1) {
								pos.x += static_cast<float>(elem.element.size.x) * gridSize * 0.5f - w.getSize().x / 2.f;
								pos.y += static_cast<float>(elem.element.size.y) * gridSize;
							} else {
								pos.x += static_cast<float>(elem.element.size.x) * gridSize + 2.f;
								pos.y += static_cast<float>(elem.element.size.y) * gridSize * 0.5f;
							}
						},
					},
					.text = std::format("{}{}", element.propertiesValues.front(), element.component.get().properties.front().suffix),
					.fontSize = 14.f,
				},
			},
		},
	};
}
