#include "boardLine.hpp"
#include "boardStorage.hpp"
#include "components/componentStore.hpp"
#include "elementState.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "vec2.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cassert>
#include <stdexcept>


using namespace squi;

void handlePosChange(Widget &w) {
	auto &storage = w.customState.get<BoardLine::Storage>();
	auto alignment = storage.getAlignment();

	if (alignment == BoardLine::Alignment::horizontal) {
		w.state.height = 20.f;
		w.state.width = static_cast<float>(std::abs(storage.startPos->x - storage.endPos->x)) * 20.f;
	} else {
		w.state.width = 20.f;
		w.state.height = static_cast<float>(std::abs(storage.startPos->y - storage.endPos->y)) * 20.f;
	}
}

BoardLine::operator squi::Child() const {
	const auto &comp = ComponentStore::components.at(2).get();

	return GestureDetector{
		.child = MsdfImage{
			.widget{
				.width = static_cast<float>(comp.width) * 20.f,
				.height = static_cast<float>(comp.height) * 20.f,
				.onInit = [&boardStorage = boardStorage, startPos = startPos](Widget &w) {
					w.customState.add(Storage{
						.boardStorage = boardStorage,
						.startPos{
							[&w](const auto &) {
								if (w.customState.get<Storage>().placed) throw std::runtime_error("Cannot replace line while it is placed!");
								handlePosChange(w);
							},
							&w,
							startPos,
						},
						.endPos{
							[&w](const auto &) {
								if (w.customState.get<Storage>().placed) throw std::runtime_error("Cannot replace line while it is placed!");
								handlePosChange(w);
							},
							&w,
							startPos,
						},
					});
					w.customState.add(StateContainer{squi::Observable<ElementState>::create()});

					w.customState.add(
						w.customState.get<StateObservable>()->observe(
							[widget = w.weak_from_this(), obs = w.customState.get<StateObservable>()->weak_from_this()](const ElementState &state) {
								if (widget.expired()) return;
								auto &img = widget.lock()->as<MsdfImage::Impl>();
								auto &storage = img.customState.get<Storage>();
								switch (state) {
									case ElementState::placed: {
										img.setColor(0xFFFFFFFF);
										storage.placed = true;
										storage.boardStorage.placeLine(widget);
										return;
									}
									case ElementState::placing: {
										img.setColor(squi::Color{1.f, 1.f, 1.f, 0.5f});
										return;
									}
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
										if (storage.placed) {
											storage.boardStorage.removeLine(widget);
										}
										return;
									}
								}
							}
						)
					);
				},
				.onUpdate = [](Widget &w) {
                    if (!w.customState.get<Storage>().selected) return;
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_DELETE)) {
						w.deleteLater();
					}
				},
				.onLayout = [](Widget &w, auto &, auto &) {
					auto &storage = w.customState.get<Storage>();
					auto alignment = storage.getAlignment();
					auto &image = w.as<MsdfImage::Impl>();

					if (alignment == Alignment::vertical) {
						image.setUv({0, 0}, {1, 0}, {1, 1}, {0, 1});
					} else {
						image.setUv({0, 1}, {0, 0}, {1, 0}, {1, 1});
					}
				},
				.onArrange = [](Widget &w, vec2 &pos) {
					auto &storage = w.customState.get<Storage>();
					auto alignment = storage.getAlignment();
					vec2 offset{
						20.f * static_cast<float>(std::min(storage.startPos->x, storage.endPos->x)),
						20.f * static_cast<float>(std::min(storage.startPos->y, storage.endPos->y)),
					};
					pos += offset;
					if (alignment == Alignment::vertical) {
						pos.x -= 10.f;
					} else {
						pos.y -= 10.f;
					}
				},
			},
			.texture = comp.texture,
			.color{1},
			.uvTopLeft{comp.uvTopLeft},
			.uvBottomRight{comp.uvBottomRight},
		},
	};
}

BoardLine::Alignment BoardLine::Storage::getAlignment() const {
	if (startPos->x == endPos->x) {
		return Alignment::vertical;
	}
	assert(startPos->y == endPos->y);
	return Alignment::horizontal;
}
