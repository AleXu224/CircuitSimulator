#pragma once
#include "widget.hpp"
#include "observer.hpp"
#include "component.hpp"
#include "boardStorage.hpp"

struct TopBar {
    // Args
	squi::Widget::Args widget{};
	std::shared_ptr<squi::Observable<std::reference_wrapper<const Component>>> componentSelectorObserver{};
	BoardStorage& boardStorage;


	struct Storage {
        // Data
		std::shared_ptr<squi::Observable<std::reference_wrapper<const Component>>> componentSelectorObserver{};
		BoardStorage &boardStorage;
	};

	operator squi::Child() const;
};