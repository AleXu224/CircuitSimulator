#pragma once
#include "widget.hpp"
#include "observer.hpp"
#include "component.hpp"

struct TopBar {
    // Args
	squi::Widget::Args widget{};
	std::shared_ptr<squi::Observable<std::reference_wrapper<const Component>>> componentSelectorObserver{};

	struct Storage {
        // Data
		std::shared_ptr<squi::Observable<std::reference_wrapper<const Component>>> componentSelectorObserver{};
	};

	operator squi::Child() const;
};