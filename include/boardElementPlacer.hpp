#pragma once

#include "boardStorage.hpp"
#include "widget.hpp"

struct BoardElementPlacer {
	// Args
	squi::Widget::Args widget{};
	BoardStorage &boardStorage;
	const Component &component;
    const squi::vec2 &viewOffset;
    squi::ChildRef board;

	struct Storage {
		// Data
        BoardStorage &boardStorage;
        const Component &component;
        const squi::vec2 &viewOffset;
        squi::ChildRef board;
        uint32_t rotation = 0;
        Coords position{};
	};

	operator squi::Child() const;
};
