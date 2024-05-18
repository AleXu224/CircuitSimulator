#pragma once
#include "boardStorage.hpp"
#include "widget.hpp"

struct BoardLinePlacer {
    // Args
    squi::Widget::Args widget{};
    Coords startPos{};
	std::reference_wrapper<BoardStorage> boardStorage;
	std::reference_wrapper<const Component> component;
	std::reference_wrapper<const squi::vec2> viewOffset;
	squi::ChildRef board;

	struct Storage {
        // Data
		Coords startPos;
        Coords endPos{startPos};
		std::reference_wrapper<BoardStorage> boardStorage;
		std::reference_wrapper<const Component> component;
		std::reference_wrapper<const squi::vec2> viewOffset;
		squi::ChildRef board;
	};

    operator squi::Child() const;
};