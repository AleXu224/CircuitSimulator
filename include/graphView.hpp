#pragma once
#include "widget.hpp"
#include "graph.hpp"

struct GraphView {
    // Args
    squi::Widget::Args widget{};
    squi::Observable<const std::vector<Graph::LineValue> &> updateData{};
    std::vector<Graph::LineValue> values{};

    struct Storage {
        // Data
        squi::Observer<const Graph::Axes &> onInfoChangeObs{};
        Graph::Axes axes{};
    };

    operator squi::Child() const;
};