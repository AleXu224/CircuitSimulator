#pragma once

#include "widget.hpp"
#include "coords.hpp"
#include "element.hpp"

struct BoardStorage;

struct BoardLine {
	// Args
    enum class Alignment{
        vertical,
        horizontal,
    };
	BoardStorage &boardStorage;
	const Element &element;

	struct Storage {
		// Data
		BoardStorage &boardStorage;
		Coords startPos;
		Coords endPos;
		bool selected = false;
		ElementId id = 0;
	};

	operator squi::Child() const;
};
