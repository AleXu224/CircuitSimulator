#pragma once
#include "boardStorage.hpp"
#include "widget.hpp"

struct BoardLinePlacer {
    // Args
    squi::Widget::Args widget{};
    Coords startPos{};
	BoardStorage &boardStorage;
	const Component &component;
	const squi::vec2 &viewOffset;
	squi::ChildRef board;

	struct Storage {
        // Data
		Coords startPos;
        Coords endPos{startPos};
		BoardStorage &boardStorage;
		const Component &component;
		const squi::vec2 &viewOffset;
		squi::ChildRef board;
	};

    operator squi::Child() const;
};