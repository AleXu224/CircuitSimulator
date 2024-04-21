#include "boardLinePlacer.hpp"
#include "config.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "utils.hpp"

using namespace squi;

BoardLinePlacer::operator squi::Child() const {
	return GestureDetector{
		.child = MsdfImage{
			.widget{
				.customState{
					Storage{
						.startPos = startPos,
						.boardStorage = boardStorage,
						.component = component,
						.viewOffset = viewOffset,
						.board = board,
					}
				},
				.onInit = [](Widget &w) {
					w.customState.add(VoidObservable{}.observe([&w]() {
						auto &storage = w.customState.get<Storage>();
						if (storage.startPos == storage.endPos) return;
						const auto endOffset = (storage.startPos - storage.endPos).abs();

						Coords size = endOffset;
						if (size.x == 0)
							size.x++;
						else
							size.y++;

						storage.boardStorage.get().placeLine(Element{
							.size{size},
							.pos{Coords::min(storage.startPos, storage.endPos)},
							.nodes{{0, 0}, endOffset},
							.component = storage.component,
						});

						storage.startPos = storage.endPos;

					}));
				},
				.onUpdate = [](Widget &w) {
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
						w.deleteLater();
						return;
					}

					auto &storage = w.customState.get<Storage>();
					if (storage.board.expired()) return;
					auto board = storage.board.lock();
					auto newPos = Utils::screenToGridRounded(
						GestureDetector::getMousePos(),
						storage.viewOffset,
						board->getPos()
					);

					const auto diff = (storage.startPos - newPos).abs();
					if (diff.x <= diff.y) {
						newPos.x = storage.startPos.x;
					} else {
						newPos.y = storage.startPos.y;
					}

					if (newPos != storage.endPos) {
						storage.endPos = newPos;
						w.reLayout();
					}
				},
				.onLayout = [](Widget &w, vec2 &maxSize, vec2 & /*minSize*/) {
					auto &storage = w.customState.get<Storage>();
					auto &image = w.as<MsdfImage::Impl>();

					if (storage.startPos.x == storage.endPos.x) {
						maxSize.x = gridSize;
						maxSize.y = static_cast<float>(std::abs(storage.startPos.y - storage.endPos.y)) * gridSize;
						image.setUv({0, 0}, {1, 0}, {1, 1}, {0, 1});
					} else {
						maxSize.y = gridSize;
						maxSize.x = static_cast<float>(std::abs(storage.startPos.x - storage.endPos.x)) * gridSize;
						image.setUv({0, 1}, {0, 0}, {1, 0}, {1, 1});
					}
				},
				.onArrange = [](Widget &w, vec2 &pos) {
					auto &storage = w.customState.get<Storage>();
					const auto minPos = Coords::min(storage.startPos, storage.endPos);
					pos += minPos.toVec() * gridSize;
					if (storage.startPos.x == storage.endPos.x) {
						pos.x -= gridSize / 2.f;
					} else {
						pos.y -= gridSize / 2.f;
					}
				},
			},
			.texture = component.get().texture,
			.color{1.f, 1.f, 1.f, 0.5f},
			.uvTopLeft{component.get().uvTopLeft},
			.uvBottomRight{component.get().uvBottomRight},
		},
	};
}
