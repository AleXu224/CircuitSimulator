#pragma once

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
		float lastValue = 0.f;
	};

	operator squi::Child() const;
};