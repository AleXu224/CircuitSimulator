#include "boardSelection.hpp"
#include "box.hpp"
#include "coords.hpp"
#include "elementState.hpp"
#include "stateContainer.hpp"
#include <algorithm>

using namespace squi;

static void handlePosChange(Widget &w) {
    auto &storage = w.customState.get<BoardSelection::Storage>();
    Coords minPos{
        std::min(storage.startPos->x, storage.endPos->x),
        std::min(storage.startPos->y, storage.endPos->y),
    };
	Coords maxPos{
		std::max(storage.startPos->x, storage.endPos->x),
		std::max(storage.startPos->y, storage.endPos->y),
	};

    w.state.width = static_cast<float>(maxPos.x - minPos.x) * 20.f;
    w.state.height = static_cast<float>(maxPos.y - minPos.y) * 20.f;
}

BoardSelection::operator squi::Child() const {
	return Box{
		.widget{
			.width = 0.f,
			.height = 0.f,
			.onInit = [startPos = startPos](Widget &w) {
				w.customState.add(Storage{
					.startPos{[](Widget &w, const Coords &) {
                        handlePosChange(w);
                    }, &w, startPos},
					.endPos{[](Widget &w, const Coords &) {
                        handlePosChange(w);
                    }, &w, startPos},
				});

                // Kind of a hack at the moment to not throw when the boardView notifies deletion
				w.customState.add(StateObservable{});
			},
			.onArrange = [](Widget &w, vec2 &pos) {
				auto &storage = w.customState.get<BoardSelection::Storage>();
				Coords minPos{
					std::min(storage.startPos->x, storage.endPos->x),
					std::min(storage.startPos->y, storage.endPos->y),
				};

                vec2 offset{
                    static_cast<float>(minPos.x) * 20.f,
                    static_cast<float>(minPos.y) * 20.f,
                };

                pos += offset;
			},
		},
		.color{0xFFFFFF22},
		.borderColor{0xFFFFFFFF},
		.borderWidth{1.f},
	};
}
