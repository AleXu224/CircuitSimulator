#include "boardElementPlacer.hpp"
#include "config.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "utils.hpp"

using namespace squi;

BoardElementPlacer::operator squi::Child() const {
	return GestureDetector{
		.child = MsdfImage{
			.widget{
				.width = static_cast<float>(component.width) * gridSize,
				.height = static_cast<float>(component.height) * gridSize,
				.customState{
					Storage{
						.boardStorage = boardStorage,
						.component = component,
						.viewOffset = viewOffset,
						.board = board,
					},
				},
				.onInit = [](Widget &w) {
					w.customState.add(VoidObservable{}.observe([&w]() {
						auto &storage = w.customState.get<Storage>();
						const auto rotatedData = Utils::rotateElement(storage.rotation, storage.component);
						storage.boardStorage.placeElement(Element{
							.size{Utils::componentSizeWithRotation(storage.component, storage.rotation)},
							.pos = storage.position,
							.rotation = storage.rotation,
							.nodes{rotatedData.newNodes},
							.component = storage.component,
							// .propertiesValues = std::vector<float>(storage.component.properties.size()),
							.propertiesValues = getProperties(0, storage.component),
						});
					}));
				},
				.onUpdate = [](Widget &w) {
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
						w.deleteLater();
						return;
					}

					auto &storage = w.customState.get<Storage>();

					int32_t rotationOffset = 0;
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_R)) rotationOffset++;
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_R, GLFW_MOD_SHIFT)) rotationOffset--;
					if (rotationOffset != 0) {
						storage.rotation += rotationOffset;
						const auto rotatedData = Utils::rotateElement(storage.rotation, storage.component);
						auto &image = w.as<MsdfImage::Impl>();
						image.setUv(rotatedData.newTopLeft, rotatedData.newTopRight, rotatedData.newBottomRight, rotatedData.newBottomLeft);

						const auto newSize = Utils::componentSizeWithRotation(storage.component, storage.rotation);
						image.state.width = static_cast<float>(newSize.x) * gridSize;
						image.state.height = static_cast<float>(newSize.y) * gridSize;
					}

					if (storage.board.expired()) return;
					auto board = storage.board.lock();
					auto newPos = Utils::screenToGridRounded(
						GestureDetector::getMousePos() - (w.getSize() / 2.f),
						storage.viewOffset,
						board->getPos()
					);

					if (newPos != storage.position) {
						storage.position = newPos;
						w.reArrange();
					}
				},
				.onArrange = [](Widget &w, vec2 &pos) {
					auto &storage = w.customState.get<Storage>();

					pos += storage.position.toVec() * gridSize;
				},
			},
			.texture = component.texture,
			.color{1.f, 1.f, 1.f, 0.5f},
			.uvTopLeft = component.uvTopLeft,
			.uvBottomRight = component.uvBottomRight,
		},
	};
}
