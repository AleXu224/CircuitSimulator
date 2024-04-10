#pragma once
#include "widget.hpp"
#include "observer.hpp"
#include "component.hpp"
#include "boardStorage.hpp"

struct TopBar {
    // Args
	squi::Widget::Args widget{};
	squi::Observable<std::reference_wrapper<const Component>> componentSelectorObserver{};
	squi::VoidObservable onRun;
	BoardStorage& boardStorage;


	struct Storage {
        // Data
		squi::Observable<std::reference_wrapper<const Component>> componentSelectorObserver{};
		BoardStorage &boardStorage;
	};

	operator squi::Child() const;
};