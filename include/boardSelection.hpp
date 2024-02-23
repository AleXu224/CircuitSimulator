#pragma once
#include "widget.hpp"
#include "coords.hpp"

struct BoardSelection {
    // Args
    Coords startPos{};

    struct Storage {
        // Data
        squi::Widget::Stateful<Coords, squi::Widget::StateImpact::RelayoutNeeded> startPos;
        squi::Widget::Stateful<Coords, squi::Widget::StateImpact::RelayoutNeeded> endPos;
    };

    operator squi::Child() const;
};