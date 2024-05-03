#pragma once
#include "acSimulation.hpp"
#include "observer.hpp"
#include "widget.hpp"

struct ACResultsViewer {
	// Args
	squi::Widget::Args widget{};
	const GraphDescriptor &graph;
	const ACSimulation &simulation;
	squi::Observable<const std::vector<ElementId> &> elementSelector{};

	struct Storage {
		// Data
	};

	operator squi::Child() const;
};