#include "boardElement.hpp"
#include "boardStorage.hpp"
#include "component.hpp"
#include "coords.hpp"
#include "element.hpp"
#include "elementState.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "stateContainer.hpp"
#include <GLFW/glfw3.h>
#include <vector>



using namespace squi;

void BoardElement::Rotate(squi::Widget &widget, uint32_t rotation, const Component &component) {
	vec2 newTopLeft{};
	vec2 newTopRight{};
	vec2 newBottomRight{};
	vec2 newBottomLeft{};
	std::vector<Coords> newNodes{};
	newNodes.reserve(component.nodes.size());
	switch (rotation % 4) {
		case 0: {
			newTopLeft = {component.uvTopLeft.x, component.uvTopLeft.y};
			newTopRight = {component.uvBottomRight.x, component.uvTopLeft.y};
			newBottomRight = {component.uvBottomRight.x, component.uvBottomRight.y};
			newBottomLeft = {component.uvTopLeft.x, component.uvBottomRight.y};
			newNodes = component.nodes;
			break;
		}
		case 1: {
			newTopLeft = {component.uvTopLeft.x, component.uvBottomRight.y};
			newTopRight = {component.uvTopLeft.x, component.uvTopLeft.y};
			newBottomRight = {component.uvBottomRight.x, component.uvTopLeft.y};
			newBottomLeft = {component.uvBottomRight.x, component.uvBottomRight.y};
			for (const auto &node: component.nodes) {
				newNodes.emplace_back(Coords{
					.x = static_cast<int>(component.height - node.y),
					.y = node.x,
				});
			}
			break;
		}
		case 2: {
			newTopLeft = {component.uvBottomRight.x, component.uvBottomRight.y};
			newTopRight = {component.uvTopLeft.x, component.uvBottomRight.y};
			newBottomRight = {component.uvTopLeft.x, component.uvTopLeft.y};
			newBottomLeft = {component.uvBottomRight.x, component.uvTopLeft.y};
			for (const auto &node: component.nodes) {
				newNodes.emplace_back(Coords{
					.x = static_cast<int>(component.width - node.x),
					.y = static_cast<int>(component.height - node.y),
				});
			}
			break;
		}
		case 3: {
			newTopLeft = {component.uvBottomRight.x, component.uvTopLeft.y};
			newTopRight = {component.uvBottomRight.x, component.uvBottomRight.y};
			newBottomRight = {component.uvTopLeft.x, component.uvBottomRight.y};
			newBottomLeft = {component.uvTopLeft.x, component.uvTopLeft.y};
			for (const auto &node: component.nodes) {
				newNodes.emplace_back(Coords{
					.x = node.y,
					.y = static_cast<int>(component.width - node.x),
				});
			}
			break;
		}
	}

	auto &image = widget.as<MsdfImage::Impl>();
	auto &element = widget.customState.get<Element>();

	if (component.width != component.height) {
		if (rotation % 2 == 0) {
			image.state.width = static_cast<float>(component.width) * 20.f;
			image.state.height = static_cast<float>(component.height) * 20.f;
			element.size.x = static_cast<int>(component.width);
			element.size.y = static_cast<int>(component.height);
			element.nodes = newNodes;
		} else {
			image.state.width = static_cast<float>(component.height) * 20.f;
			image.state.height = static_cast<float>(component.width) * 20.f;
			element.size.y = static_cast<int>(component.width);
			element.size.x = static_cast<int>(component.height);
			element.nodes = newNodes;
		}
	}

	image.setUv(newTopLeft, newTopRight, newBottomRight, newBottomLeft);
}

BoardElement::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.boardStorage = boardStorage,
	});

	return GestureDetector{
		.onUpdate = [storage](GestureDetector::Event event) {
			if (storage->selected) {
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_DELETE) || GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) event.widget.deleteLater();
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) event.widget.customState.get<StateObservable>()->notify(ElementState::unselected);
			}
		},
		.child = MsdfImage{
			.widget{
				.customState{element},
				.onInit = [storage = storage, placed = placed](Widget &w) {
					auto &element = w.customState.get<Element>();
					element.size = {
						.x = static_cast<int32_t>(element.component.get().width),
						.y = static_cast<int32_t>(element.component.get().height),
					};
					w.state.width = static_cast<float>(element.component.get().width) * 20.f;
					w.state.height = static_cast<float>(element.component.get().height) * 20.f;

					BoardElement::Rotate(w, element.rotation, element.component);
					w.customState.add(StateContainer{squi::Observable<ElementState>::create()});

					w.customState.add(
						w.customState.get<StateObservable>()->observe(
							[storage = storage, widget = w.weak_from_this(), obs = w.customState.get<StateObservable>()->weak_from_this()](const ElementState &state) {
								if (widget.expired()) return;
								auto &img = widget.lock()->as<MsdfImage::Impl>();
								switch (state) {
									case ElementState::placed: {
										img.setColor(0xFFFFFFFF);
										storage->placed = true;
										storage->boardStorage.placeElement(widget);
										return;
									}
									case ElementState::placing: {
										img.setColor(squi::Color{1.f, 1.f, 1.f, 0.5f});
										return;
									}
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
										if (storage->placed) {
											storage->boardStorage.removeElement(widget);
										}
										return;
									}
								}
							}
						)
					);

					if (placed) {
						w.customState.get<StateObservable>()->notify(ElementState::placed);
					}
				},
				.onArrange = [](Widget &w, auto &pos) {
					auto &elemPos = w.customState.get<Element>().pos;

					pos += vec2(static_cast<float>(elemPos.x), static_cast<float>(elemPos.y)) * 20.f;
				},
			},
			.texture = element.component.get().texture,
			.color{1.f, 1.f, 1.f, 0.5f},
			.uvTopLeft{element.component.get().uvTopLeft},
			.uvBottomRight{element.component.get().uvBottomRight},
		},
	};
}
