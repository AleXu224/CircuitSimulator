#pragma once

#include "coords.hpp"
#include "widget.hpp"

struct BoardStorage;

struct BoardLine {
	// Args
    enum class Alignment{
        vertical,
        horizontal,
    };
	BoardStorage &boardStorage;
	Coords startPos{};

	struct Storage {
		// Data
		BoardStorage &boardStorage;
		squi::Widget::Stateful<Coords, squi::Widget::StateImpact::RelayoutNeeded> startPos;
		squi::Widget::Stateful<Coords, squi::Widget::StateImpact::RelayoutNeeded> endPos;
		bool placed = false;
		bool selected = false;

		[[nodiscard]] Alignment getAlignment() const;
	};

	operator squi::Child() const;
};
