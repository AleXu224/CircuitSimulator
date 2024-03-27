#pragma once

#include "component.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "element.hpp"

struct BoardStorage;

struct BoardElement {
	// Args
	squi::Widget::Args widget{};
	Element element;
	BoardStorage &boardStorage;
	bool placed = false;

	static void Rotate(squi::Widget &widget, uint32_t rotation, const Component &component);

	struct Storage {
		// Data
		squi::vec2 dragStartPos{};
		bool selected = false;
		bool placed = false;
		BoardStorage &boardStorage;
	};

	operator squi::Child() const;
};