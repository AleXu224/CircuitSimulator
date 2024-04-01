#pragma once

#include "component.hpp"
#include "widget.hpp"
#include "element.hpp"

struct BoardStorage;

struct BoardElement {
	// Args
	squi::Widget::Args widget{};
	const Element &element;
	BoardStorage &boardStorage;

	struct Storage {
		// Data
		bool selected = false;
		BoardStorage &boardStorage;
		uint32_t elementId = 0;
	};

	operator squi::Child() const;
};