#pragma once

#include "../extern/portable-file-dialogs/portable-file-dialogs.h"

#include "boardLine.hpp"

#include "boardElement.hpp"
#include "components/componentStore.hpp"
#include "connection.hpp"
#include "coords.hpp"
#include "element.hpp"
#include "fstream"
#include "msdfImage.hpp"
#include "observer.hpp"
#include "saveData.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <algorithm>
#include <filesystem>
#include <functional>
#include <print>
#include <ranges>
#include <unordered_map>
#include <vector>


struct BoardStorage {
	// std::vector<Line> lines{};
	std::unordered_map<Coords, std::vector<squi::ChildRef>> lineTiles{};
	std::vector<squi::ChildRef> lines{};
	std::unordered_map<Coords, std::pair<squi::Child, std::vector<Connection>>> nodes{};
	std::unordered_map<Coords, squi::ChildRef> elementTiles{};
	std::vector<squi::ChildRef> elements{};

	using ObservableType = squi::Observable<const squi::Child &>;
	using BoardObservable = std::shared_ptr<ObservableType>;
	BoardObservable boardObservable = ObservableType::create();

	std::shared_ptr<squi::VoidObservable> clearObservable = squi::VoidObservable::create();

	void save() {
		SaveData ret{};
		ret.elements.reserve(elements.size());
		for (const auto &elem: elements) {
			if (elem.expired()) continue;
			auto w = elem.lock();
			auto &e = w->customState.get<Element>();
			ret.elements.emplace_back(ElementSaveData{
				.id = e.id,
				.type = e.component.get().id,
				.posX = e.pos.x,
				.posY = e.pos.y,
				.rotation = e.rotation,
			});
		}

		for (const auto &line: lines) {
			if (line.expired()) continue;
			auto w = line.lock();
			auto &e = w->customState.get<Element>();
			auto &s = w->customState.get<BoardLine::Storage>();
			ret.lines.emplace_back(LineSaveData{
				.id = e.id,
				.startPosX = s.startPos->x,
				.startPosY = s.startPos->y,
				.endPosX = s.endPos->x,
				.endPosY = s.endPos->y,
			});
		}

		auto serialized = ret.serialize();

		auto pathStr = pfd::save_file(
						   "Save",
						   {},
						   {
							   "Circuit Simulator Save File (*.sqcs)",
							   "*.sqcs",
							   "All Files (*.*)",
							   "*",
						   }
		)
						   .result();

		if (!pathStr.empty()) {
			std::filesystem::path path(pathStr);
			std::println("Saving to: {}", path.string());
			std::ofstream out(path, std::ios::trunc | std::ios::out | std::ios::binary);
			out.write(std::bit_cast<char *>(serialized.data()), static_cast<int64_t>(serialized.size()));
			out.close();
		}
	}

	void load() {
		auto pathStr = pfd::open_file(
						   "Load",
						   {},
						   {
							   "Circuit Simulator Save File (*.sqcs)",
							   "*.sqcs",
							   "All Files (*.*)",
							   "*",
						   }
		)
						   .result();

		if (!pathStr.empty()) {
			std::filesystem::path path(pathStr.front());
			std::println("Loading save: {}", path.string());
			std::ifstream in(path, std::ios::in | std::ios::binary);
			if (!in.is_open()) {
				std::println("Failed to open save file");
				return;
			}
			std::stringstream contents{};
			contents << in.rdbuf();
			in.close();

			SaveData data{};
			auto contentsView = contents.view();
			data.deserialize(std::span<const std::byte>(
				std::bit_cast<const std::byte *>(&*contentsView.begin()),
				contentsView.size()
			));

			elements.clear();
			elementTiles.clear();
			lines.clear();
			lineTiles.clear();
			nodes.clear();
			clearObservable->notify();

			uint32_t maxId = 0;

			const auto &lineComponent = ComponentStore::components.at(0).get();
			for (auto &elem: data.elements) {
				maxId = std::max(maxId, elem.id);
				boardObservable->notify(BoardElement{
					.element{
						.id = elem.id,
						.pos{
							.x = elem.posX,
							.y = elem.posY,
						},
						.rotation = elem.rotation,
						.component = ComponentStore::components.at(elem.type).get(),
					},
					.boardStorage = *this,
					.placed = true,
				});
			}
			for (auto &line: data.lines) {
				maxId = std::max(maxId, line.id);
				boardObservable->notify(BoardLine{
					.boardStorage = *this,
					.startPos{line.startPosX, line.startPosY},
					.endPos{Coords{line.endPosX, line.endPosY}},
					.elem{Element{
						.id = line.id,
						.size{
							.x = static_cast<int32_t>(lineComponent.width),
							.y = static_cast<int32_t>(lineComponent.height),
						},
						.component = lineComponent,
						.type = ElementType::Line,
					}},
				});
			}

			Element::idCounter = maxId;
		}
	}

	bool isElementOverlapping(const Element &elem) {
		auto elemSize = elem.size;
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				if (elementTiles.contains(Coords{.x = i, .y = j}))
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
			.texture{ComponentStore::components.at(1).get().texture}
		};
	}

	void placeLine(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &storage = child.lock()->customState.get<BoardLine::Storage>();
		auto alignment = storage.getAlignment();
		auto modifiedVal = alignment == BoardLine::Alignment::horizontal ? &Coords::x : &Coords::y;
		Coords minCoords = Coords::min(storage.startPos, storage.endPos);
		Coords maxCoords = Coords::max(storage.startPos, storage.endPos);
		auto distance = alignment == BoardLine::Alignment::horizontal ? maxCoords.x - minCoords.x : maxCoords.y - minCoords.y;
		const auto initialVal = std::invoke(modifiedVal, minCoords);

		for (auto i: std::views::iota(0, distance)) {
			std::invoke(modifiedVal, minCoords) = initialVal + i;
			lineTiles[minCoords].emplace_back(child);
		}
		auto &element = child.lock()->customState.get<Element>();
		nodes[storage.startPos].second.emplace_back(Connection{
			.nodeIndex = 0,
			.element = element,
			.widget = child,
		});
		nodes[storage.startPos].first = createNodeChild(storage.startPos);
		if (*storage.startPos != *storage.endPos) {
			nodes[storage.endPos].second.emplace_back(Connection{
				.nodeIndex = 1,
				.element = element,
				.widget = child,
			});
			nodes[storage.endPos].first = createNodeChild(storage.endPos);
		}
		lines.emplace_back(child);
	}

	void removeLine(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &storage = child.lock()->customState.get<BoardLine::Storage>();
		auto alignment = storage.getAlignment();
		auto modifiedVal = alignment == BoardLine::Alignment::horizontal ? &Coords::x : &Coords::y;
		Coords minCoords = Coords::min(storage.startPos, storage.endPos);
		Coords maxCoords = Coords::max(storage.startPos, storage.endPos);
		auto distance = alignment == BoardLine::Alignment::horizontal ? maxCoords.x - minCoords.x : maxCoords.y - minCoords.y;
		const auto initialVal = std::invoke(modifiedVal, minCoords);

		for (auto i: std::views::iota(0, distance)) {
			std::invoke(modifiedVal, minCoords) = initialVal + i;
			std::erase_if(lineTiles[minCoords], [&](auto &val) -> bool {
				if (val.expired()) return true;
				return child.lock() == val.lock();
			});
		}
		std::erase_if(nodes[storage.startPos].second, [&](auto &elem) -> bool {
			if (elem.widget.expired()) return true;
			return elem.widget.lock() == child.lock();
		});

		if (*storage.startPos != *storage.endPos) {
			std::erase_if(nodes[storage.endPos].second, [&](auto &elem) -> bool {
				if (elem.widget.expired()) return true;
				return elem.widget.lock() == child.lock();
			});
		}
		lines.erase(
			std::remove_if(lines.begin(), lines.end(), [&](const squi::ChildRef &elem) {
				if (elem.expired()) return true;
				return child.lock() == elem.lock();
			}),
			lines.end()
		);
	}

	void placeElement(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &elem = child.lock()->customState.get<Element>();
		auto elemSize = elem.size;
		for (int i = elem.pos.x; i < elemSize.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elemSize.y + elem.pos.y; j++) {
				elementTiles[Coords{i, j}] = child;
			}
		}
		for (const auto &[index, nodeCoords]: elem.nodes | std::views::enumerate) {
			auto &nodePair = nodes[elem.pos + nodeCoords];
			if (!nodePair.first) {
				nodePair.first = createNodeChild(elem.pos + nodeCoords);
			}
			nodePair.second.emplace_back(Connection{
				.nodeIndex = size_t(index),
				.element = elem,
				.widget = child,
			});
		}
		elements.emplace_back(child);
	}

	void removeElement(const squi::ChildRef &child) {
		if (child.expired()) return;
		auto &elem = child.lock()->customState.get<Element>();
		for (int i = elem.pos.x; i < elem.size.x + elem.pos.x; i++) {
			for (int j = elem.pos.y; j < elem.size.y + elem.pos.y; j++) {
				if (auto it = elementTiles.find(Coords{.x = i, .y = j}); it != elementTiles.end()) {
					elementTiles.erase(it);
				}
			}
		}
		for (const auto &nodeCoords: elem.nodes) {
			auto &vec = nodes[elem.pos + nodeCoords].second;
			std::erase_if(vec, [&child](auto &elem) -> bool {
				if (elem.widget.expired()) return true;
				return elem.widget.lock() == child.lock();
			});
		}
		elements.erase(
			std::remove_if(elements.begin(), elements.end(), [&](const squi::ChildRef &elem) {
				if (elem.expired()) return true;
				return child.lock() == elem.lock();
			}),
			elements.end()
		);
	}
};