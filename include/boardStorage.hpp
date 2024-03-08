#pragma once

#include "boardLine.hpp"
#include "components/componentStore.hpp"
#include "coords.hpp"
#include "element.hpp"
#include "msdfImage.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <algorithm>
#include <functional>
#include <ranges>
#include <unordered_map>
#include <vector>


struct BoardStorage {
	// std::vector<Line> lines{};
	std::unordered_map<Coords, std::vector<squi::ChildRef>> lines{};
	std::unordered_map<Coords, std::pair<squi::Child, std::vector<squi::ChildRef>>> nodes{};
	std::unordered_map<Coords, squi::ChildRef> board{};

	bool isElementOverlapping(const Element &elem) {
		auto elemSize = elem.size;
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				if (board.contains(Coords{.x = i, .y = j}))
					return true;
			}
		}

		return false;
	}

	static squi::Child createNodeChild(const Coords &coords) {
		return MsdfImage{
			.widget{
				.width = 20.f,
				.height = 20.f,
				.onArrange = [coords](squi::Widget & /*w*/, squi::vec2 &pos) {
					squi::vec2 offset(static_cast<float>(coords.x) * 20.f, static_cast<float>(coords.y) * 20.f);
					offset -= 10.f;
					pos += offset;
				},
			},
			.texture{ComponentStore::components.at(3).get().texture}
		};
	}

	void placeLine(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &storage = child.lock()->customState.get<BoardLine::Storage>();
		auto alignment = storage.getAlignment();
		auto modifiedVal = alignment == BoardLine::Alignment::horizontal ? &Coords::x : &Coords::y;
		Coords minCoords{
			std::min(storage.startPos->x, storage.endPos->x),
			std::min(storage.startPos->y, storage.endPos->y),
		};
		Coords maxCoords{
			std::max(storage.startPos->x, storage.endPos->x),
			std::max(storage.startPos->y, storage.endPos->y),
		};
		auto distance = alignment == BoardLine::Alignment::horizontal ? maxCoords.x - minCoords.x : maxCoords.y - minCoords.y;
		const auto initialVal = std::invoke(modifiedVal, minCoords);

		for (auto i: std::views::iota(0, distance)) {
			std::invoke(modifiedVal, minCoords) = initialVal + i;
			lines[minCoords].emplace_back(child);
		}
		nodes[storage.startPos].second.emplace_back(child);
		nodes[storage.startPos].first = createNodeChild(storage.startPos);
		if (*storage.startPos != *storage.endPos) {
			nodes[storage.endPos].second.emplace_back(child);
			nodes[storage.endPos].first = createNodeChild(storage.endPos);
		}
	}

	void removeLine(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &storage = child.lock()->customState.get<BoardLine::Storage>();
		auto alignment = storage.getAlignment();
		auto modifiedVal = alignment == BoardLine::Alignment::horizontal ? &Coords::x : &Coords::y;
		Coords minCoords{
			std::min(storage.startPos->x, storage.endPos->x),
			std::min(storage.startPos->y, storage.endPos->y),
		};
		Coords maxCoords{
			std::max(storage.startPos->x, storage.endPos->x),
			std::max(storage.startPos->y, storage.endPos->y),
		};
		auto distance = alignment == BoardLine::Alignment::horizontal ? maxCoords.x - minCoords.x : maxCoords.y - minCoords.y;
		const auto initialVal = std::invoke(modifiedVal, minCoords);

		// FIXME: crash here because of a negative distance
		for (auto i: std::views::iota(0, distance)) {
			std::invoke(modifiedVal, minCoords) = initialVal + i;
			std::erase_if(lines[minCoords], [&](auto &val) -> bool {
				if (val.expired()) return true;
				return child.lock() == val.lock();
			});
		}
		std::erase_if(nodes[storage.startPos].second, [&](auto &elem) -> bool {
			if (elem.expired()) return true;
			return elem.lock() == child.lock();
		});

		if (*storage.startPos != *storage.endPos) {
			std::erase_if(nodes[storage.endPos].second, [&](auto &elem) -> bool {
				if (elem.expired()) return true;
				return elem.lock() == child.lock();
			});
		}
	}

	void placeElement(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &elem = child.lock()->customState.get<Element>();
		auto elemSize = elem.size;
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				board[Coords{i, j}] = child;
			}
		}
		for (const auto &nodeCoords: elem.nodes) {
			auto &nodePair = nodes[elem.pos + nodeCoords];
			if (!nodePair.first) {
				nodePair.first = createNodeChild(elem.pos + nodeCoords);
			}
			nodePair.second.emplace_back(child);
		}
	}

	void removeElement(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &elem = child.lock()->customState.get<Element>();
		for (int i = elem.pos.x; i < elem.size.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elem.size.y + elem.pos.y; j++) {
				if (auto it = board.find(Coords{.x = i, .y = j}); it != board.end()) {
					board.erase(it);
				}
			}
		}
		for (const auto &nodeCoords: elem.nodes) {
			auto &vec = nodes[elem.pos + nodeCoords].second;
			std::erase_if(vec, [&child](auto &elem) -> bool {
				if (elem.expired()) return true;
				return elem.lock() == child.lock();
			});
		}
	}
};