#include "boardLine.hpp"
#include "boardStorage.hpp"
#include "components/componentStore.hpp"
#include "config.hpp"
#include "element.hpp"
#include "elementState.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "vec2.hpp"
#include <GLFW/glfw3.h>
#include <cassert>


using namespace squi;

BoardLine::operator squi::Child() const {
	const auto &comp = ComponentStore::components.at(0).get();

	return GestureDetector{
		.onUpdate = [](GestureDetector::Event event) {
			auto &storage = event.widget.customState.get<Storage>();
			if (storage.selected) {
				if (GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_DELETE) || GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
					storage.boardStorage.removeLine(storage.id);
				}
			}
		},
		.child = MsdfImage{
			.widget{
				.width = static_cast<float>(element.size.x) * 20.f,
				.height = static_cast<float>(element.size.y) * 20.f,
				.customState{Storage{
					.boardStorage = boardStorage,
					.startPos = element.pos + element.nodes.at(0),
					.endPos = element.pos + element.nodes.at(1),
					.selected = false,
					.id = element.id,
				}},
				.onInit = [](Widget &w) {
					w.customState.add(StateObservable{});

					w.customState.add(
						w.customState.get<StateObservable>().observe(
							[widget = w.weak_from_this()](const ElementState &state) {
								if (widget.expired()) return;
								auto &img = widget.lock()->as<MsdfImage::Impl>();
								auto &storage = img.customState.get<Storage>();
								switch (state) {
									case ElementState::selected: {
										storage.selected = true;
										img.setColor(0xAAAAFFFF);
										return;
									}
									case ElementState::unselected: {
										storage.selected = false;
										img.setColor(0xFFFFFFFF);
										return;
									}
									case ElementState::removed: {
										storage.boardStorage.removeLine(storage.id);
										return;
									}
								}
							}
						)
					);
				},
				.onLayout = [](Widget &w, auto &, auto &) {
					auto &storage = w.customState.get<Storage>();
					auto &image = w.as<MsdfImage::Impl>();

					if (storage.startPos.x == storage.endPos.x) {
						image.setUv({0, 0}, {1, 0}, {1, 1}, {0, 1});
					} else {
						image.setUv({0, 1}, {0, 0}, {1, 0}, {1, 1});
					}
				},
				.onArrange = [](Widget &w, vec2 &pos) {
					auto &storage = w.customState.get<Storage>();
					pos += Coords::min(storage.startPos, storage.endPos).toVec() * gridSize;

					if (storage.startPos.x == storage.endPos.x) {
						pos.x -= gridSize / 2.f;
					} else {
						pos.y -= gridSize / 2.f;
					}
				},
			},
			.texture = comp.texture,
			.color{1.f, 1.f, 1.f, 1.f},
			.uvTopLeft{comp.uvTopLeft},
			.uvBottomRight{comp.uvBottomRight},
		},
	};
}
