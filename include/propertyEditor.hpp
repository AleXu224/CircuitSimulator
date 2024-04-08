#pragma once

#include "element.hpp"
#include "observer.hpp"
#include "widget.hpp"


struct PropertyEditor {
	// Args
	squi::Widget::Args widget{};
	const Element &element;

	struct Storage {
		// Data
		std::vector<float> values{};
		squi::Observable<bool> closeObs{};
		const Element &element;
	};

	operator squi::Child() const;
};