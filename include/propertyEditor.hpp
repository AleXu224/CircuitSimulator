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
		size_t propIndex;
		std::vector<PropertyVariant> props;
		squi::Observable<bool> closeObs{};
		uint64_t focusIndex = 0;
		std::vector<squi::Observable<bool>> focusObservables{};
		squi::VoidObservable saveObs{};
		squi::VoidObservable propIndexChanged{};
		const Element &element;
	};

	operator squi::Child() const;
};