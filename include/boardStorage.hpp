#pragma once

#include "coords.hpp"
#include "element.hpp"
#include "line.hpp"
#include "widget.hpp"
#include <unordered_map>
#include <vector>


struct BoardStorage {
	std::vector<Line> lines{};
	std::unordered_map<Coords, squi::ChildRef> board{};

	bool isElementOverlapping(const Element &elem) {
		auto elemSize = elem.getSize();
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				if (board.contains(Coords{.x = i, .y = j}))
					return true;
			}
		}

		return false;
	}

	void placeElement(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &elem = child.lock()->customState.get<Element>();
		auto elemSize = elem.getSize();
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				board[Coords{i, j}] = child;
			}
		}
	}

	void removeElement(const Element &elem) {
		auto elemSize = elem.getSize();
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				if (auto it = board.find(Coords{.x = i, .y = j}); it != board.end()) {
					board.erase(it);
				}
			}
		}
	}
};