#pragma once

#include "connection.hpp"
#include "coords.hpp"
#include "element.hpp"
#include "widget.hpp"
#include <functional>
#include <print>
#include <unordered_map>
#include <vector>


struct BoardStorage {
	// std::vector<Line> lines{};
	std::vector<ElementData> lines{};
	std::vector<ElementData> elements{};

	std::unordered_map<Coords, std::vector<ElementId>> lineTiles{};
	std::unordered_map<Coords, std::vector<ElementId>> elementTiles{};

	std::unordered_map<Coords, ConnectionNode> connections{};

	void save();
	void load();

	static std::vector<ElementData>::const_iterator lower_bound(const std::vector<ElementData> &rng, ElementId id);
	static std::vector<ElementData>::const_iterator upper_bound(const std::vector<ElementData> &rng, ElementId id);
	[[nodiscard]] std::optional<std::reference_wrapper<const ElementData>> getElement(ElementId id) const;
	[[nodiscard]] std::optional<std::reference_wrapper<const ElementData>> getLine(ElementId id) const;

	bool isElementOverlapping(const Element &elem);

	[[nodiscard]] std::optional<std::reference_wrapper<const ElementData>> getClosestElementData(const std::vector<ElementId> &ids, const Coords &coords) const;

	static squi::Child createNodeChild(const Coords &coords);

	static void ensureConnectionNode(ConnectionNode &node, const Coords &coords);

	void placeLine(const Element &elem);

	void removeLine(ElementId id);

	void placeElement(const Element &elem);

	void removeElement(ElementId id);
};