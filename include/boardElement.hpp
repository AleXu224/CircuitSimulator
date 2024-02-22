#pragma once

#include "boardStorage.hpp"
#include "component.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "coords.hpp"

struct BoardElement {
	// Args
	squi::Widget::Args widget{};
	Coords position{
		.x = 0,
		.y = 0,
	};
	uint32_t rotation = 0;
	const Component &component;
	BoardStorage &boardStorage;

	static void Rotate(squi::Widget &widget, uint32_t rotation, const Component &component);

	struct Storage {
		// Data
		squi::vec2 dragStartPos{};
		bool selected = false;
		bool placed = false;
		const Component &component;
		BoardStorage &boardStorage;
	};

	operator squi::Child() const;
};