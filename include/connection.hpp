#pragma once

#include "element.hpp"
#include "widget.hpp"


struct ElementData {
	Element element;
	squi::Child widget;
};

struct LineData {
	Coords startPos;
	Coords endPos;
	Element element;
	squi::Child widget;
};

struct Connection {
	size_t nodeIndex;
	ElementId elementId;

	bool operator==(const Connection &other) const {
		return nodeIndex == other.nodeIndex && elementId == other.elementId;
	}
	bool operator!=(const Connection &other) const { return !(*this == other); }
};

struct ConnectionNode {
	squi::Child widget;
	std::vector<Connection> connections{};
};