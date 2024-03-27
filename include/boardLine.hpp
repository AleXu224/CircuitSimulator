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
	Coords startPos{};
	std::optional<Coords> endPos{};
	std::optional<Element> elem{};

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
