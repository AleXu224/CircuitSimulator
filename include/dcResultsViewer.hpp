#pragma once
#include "dcSimulation.hpp"
#include "observer.hpp"
#include "widget.hpp"

struct DCResultsViewer {
	// Args
	squi::Widget::Args widget{};
    const GraphDescriptor &graph;
	const DCSimulation &simulation;
	squi::Observable<const std::vector<ElementId>&> elementSelector{};

	struct Storage {
		// Data
	};

	operator squi::Child() const;
};