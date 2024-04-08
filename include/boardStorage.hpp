#pragma once

#include "../extern/portable-file-dialogs/portable-file-dialogs.h"

#include "boardLine.hpp"

#include "boardElement.hpp"
#include "components/componentStore.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "coords.hpp"
#include "element.hpp"
#include "fstream"
#include "msdfImage.hpp"
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
#include "utils.hpp"


struct BoardStorage {
	// std::vector<Line> lines{};
	std::vector<ElementData> lines{};
	std::vector<ElementData> elements{};

	std::unordered_map<Coords, std::vector<ElementId>> lineTiles{};
	std::unordered_map<Coords, ElementId> elementTiles{};

	std::unordered_map<Coords, ConnectionNode> connections{};

	void save() {
		SaveData ret{};
		ret.elements.reserve(elements.size());
		for (const auto &elem: elements) {
			const auto &e = elem.element;
			ret.elements.emplace_back(ElementSaveData{
				.id = e.id,
				.type = e.component.get().id,
				.posX = e.pos.x,
				.posY = e.pos.y,
				.rotation = e.rotation,
				.propertyIndex = static_cast<uint32_t>(ret.properties.size()),
				.propertyCount = static_cast<uint32_t>(e.propertiesValues.size()),
			});

			for (const auto& prop: e.propertiesValues) {
				ret.properties.emplace_back(prop);
			}
		}

		for (const auto &line: lines) {
			const auto &e = line.element;
			const auto &startPos = e.nodes.at(0) + e.pos;
			const auto &endPos = e.nodes.at(1) + e.pos;
			ret.lines.emplace_back(LineSaveData{
				.id = e.id,
				.startPosX = startPos.x,
				.startPosY = startPos.y,
				.endPosX = endPos.x,
				.endPosY = endPos.y,
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
			connections.clear();

			uint32_t maxId = 0;

			const auto &lineComponent = ComponentStore::components.at(0).get();
			for (auto &elem: data.elements) {
				maxId = std::max(maxId, elem.id);

				const auto &component = ComponentStore::components.at(elem.type).get();

				placeElement(Element{
					.id = elem.id,
					.size{Utils::componentSizeWithRotation(component, elem.rotation)},
					.pos{elem.posX, elem.posY},
					.rotation = elem.rotation,
					.nodes{Utils::rotateElement(elem.rotation, component).newNodes},
					.component = component,
					.propertiesValues = std::vector<float>(
						data.properties.begin() + elem.propertyIndex,
						data.properties.begin() + elem.propertyIndex + elem.propertyCount
					),
				});
			}
			for (auto &line: data.lines) {
				maxId = std::max(maxId, line.id);
				auto startPos = Coords{line.startPosX, line.startPosY};
				auto endPos = Coords{line.endPosX, line.endPosY};

				auto endOffset = (startPos - endPos).abs();

				Coords size = endOffset;
				if (size.x == 0)
					size.x++;
				else
					size.y++;

				placeLine(Element{
					.id = line.id,
					.size{size},
					.pos{Coords::min(startPos, endPos)},
					.nodes{{0, 0}, endOffset},
					.component = lineComponent,
					.type = ElementType::Line,
				});
			}

			Element::idCounter = maxId;
		}
	}

	static std::vector<ElementData>::const_iterator lower_bound(const std::vector<ElementData> &rng, ElementId id) {
		return std::lower_bound(rng.begin(), rng.end(), id, [](const ElementData &e, ElementId id) {
			return e.element.id < id;
		});
	}
	static std::vector<ElementData>::const_iterator upper_bound(const std::vector<ElementData> &rng, ElementId id) {
		return std::upper_bound(rng.begin(), rng.end(), id, [](ElementId id, const ElementData &e) {
			return id < e.element.id;
		});
	}
	[[nodiscard]] std::optional<std::reference_wrapper<const ElementData>> getElement(ElementId id) const {
		auto it = lower_bound(elements, id);
		if (it == elements.end()) return {};
		if (it->element.id == id) return {std::reference_wrapper<const ElementData>(*it)};
		return {};
	}
	[[nodiscard]] std::optional<std::reference_wrapper<const ElementData>> getLine(ElementId id) const {
		auto it = lower_bound(lines, id);
		if (it == lines.end()) return {};
		if (it->element.id == id) return {std::reference_wrapper<const ElementData>(*it)};
		return {};
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
				.width = gridSize,
				.height = gridSize,
				.onArrange = [coords](squi::Widget & /*w*/, squi::vec2 &pos) {
					squi::vec2 offset(static_cast<float>(coords.x) * gridSize, static_cast<float>(coords.y) * gridSize);
					offset -= gridSize / 2.f;
					pos += offset;
				},
			},
			.texture{ComponentStore::components.at(1).get().texture}
		};
	}

	static void ensureConnectionNode(ConnectionNode &node, const Coords &coords) {
		if (!node.widget) {
			node.widget = createNodeChild(coords);
		}
	}

	void placeLine(const Element &elem) {
		for (auto x: std::views::iota(elem.pos.x) | std::views::take(elem.size.x)) {
			for (auto y: std::views::iota(elem.pos.y) | std::views::take(elem.size.y)) {
				lineTiles[Coords{x, y}].emplace_back(elem.id);
			}
		}

		for (const auto &[index, node]: elem.nodes | std::views::enumerate) {
			auto &connectionNode = connections[elem.pos + node];
			connectionNode.connections.emplace_back(index, elem.id);
			ensureConnectionNode(connectionNode, node + elem.pos);
		}

		lines.emplace(
			upper_bound(lines, elem.id),
			elem,
			BoardLine{
				.boardStorage = *this,
				.element = elem,
			}
		);
	}

	void removeLine(ElementId id) {
		const auto it = lower_bound(lines, id);
		if (it == lines.end() || it->element.id != id) return;
		it->widget->reDraw();
		const auto &elem = it->element;
		for (auto x: std::views::iota(elem.pos.x) | std::views::take(elem.size.x)) {
			for (auto y: std::views::iota(elem.pos.y) | std::views::take(elem.size.y)) {
				auto &tiles = lineTiles[Coords{x, y}];
				tiles.erase(std::ranges::find(tiles, id));
			}
		}

		for (const auto &[index, node]: elem.nodes | std::views::enumerate) {
			std::erase(connections[elem.pos + node].connections, Connection{static_cast<size_t>(index), id});
		}

		lines.erase(it);
	}

	void placeElement(const Element &elem) {
		for (auto x: std::views::iota(elem.pos.x) | std::views::take(elem.size.x)) {
			for (auto y: std::views::iota(elem.pos.y) | std::views::take(elem.size.y)) {
				elementTiles[Coords{x, y}] = elem.id;
			}
		}

		for (const auto &[index, node]: elem.nodes | std::views::enumerate) {
			auto &connectionNode = connections[elem.pos + node];
			connectionNode.connections.emplace_back(index, elem.id);
			ensureConnectionNode(connectionNode, node + elem.pos);
		}

		elements.emplace(
			upper_bound(elements, elem.id),
			elem,
			BoardElement{
				.element = elem,
				.boardStorage = *this,
			}
		);
	}

	void removeElement(ElementId id) {
		const auto it = lower_bound(elements, id);
		if (it == elements.end()) return;
		it->widget->reDraw();
		const auto &elem = it->element;
		for (auto x: std::views::iota(elem.pos.x) | std::views::take(elem.size.x)) {
			for (auto y: std::views::iota(elem.pos.y) | std::views::take(elem.size.y)) {
				elementTiles.erase(elementTiles.find(Coords{x, y}));
			}
		}

		for (const auto &[index, node]: elem.nodes | std::views::enumerate) {
			std::erase(connections[elem.pos + node].connections, Connection{static_cast<size_t>(index), id});
		}

		elements.erase(it);
	}
};