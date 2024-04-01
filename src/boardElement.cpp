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
#include "stateContainer.hpp"
#include <GLFW/glfw3.h>


using namespace squi;

BoardElement::operator squi::Child() const {
	return GestureDetector{
		.onUpdate = [](GestureDetector::Event event) {
			auto &storage = event.widget.customState.get<Storage>();
			if (storage.selected) {
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_DELETE) || GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
					storage.boardStorage.removeElement(storage.elementId);
				}
			}
		},
		.child = MsdfImage{
			.widget{
				.width = static_cast<float>(element.size.x) * gridSize,
				.height = static_cast<float>(element.size.y) * gridSize,
				.customState{Storage{
					.selected = false,
					.boardStorage = boardStorage,
					.elementId = element.id,
				}},
				.onInit = [rotationData = Utils::rotateElement(element.rotation, element.component)](Widget &w) {
					auto &image = w.as<MsdfImage::Impl>();
					image.setUv(rotationData.newTopLeft, rotationData.newTopRight, rotationData.newBottomRight, rotationData.newBottomLeft);
					
					w.customState.add(StateObservable{});

					w.customState.add(
						w.customState.get<StateObservable>().observe(
							[widget = w.weak_from_this()](const ElementState &state) {
								if (widget.expired()) return;
								auto &storage = widget.lock()->customState.get<Storage>();
								auto &img = widget.lock()->as<MsdfImage::Impl>();
								switch (state) {
									case ElementState::selected: {
										img.setColor(0xAAAAFFFF);
										storage.selected = true;
										return;
									}
									case ElementState::unselected: {
										storage.selected = false;
										img.setColor(0xFFFFFFFF);
										return;
									}
									case ElementState::removed: {
										storage.boardStorage.removeElement(storage.elementId);
										return;
									}
								}
							}
						)
					);
				},
				.onArrange = [](Widget &w, auto &pos) {
					const auto &storage = w.customState.get<Storage>();
					const auto &elem = storage.boardStorage.getElement(storage.elementId)->get();

					pos += vec2(static_cast<float>(elem.element.pos.x), static_cast<float>(elem.element.pos.y)) * 20.f;
				},
			},
			.texture = element.component.get().texture,
			.color{1.f, 1.f, 1.f, 1.f},
			.uvTopLeft{element.component.get().uvTopLeft},
			.uvBottomRight{element.component.get().uvBottomRight},
		},
	};
}
