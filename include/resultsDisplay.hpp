#pragma once

#include "observer.hpp"
#include "widget.hpp"

struct ResultsDisplay {
    // Args
    squi::Widget::Args widget{};
    squi::VoidObservable destroyObs{};
    squi::Child child{};

    struct Storage {
        // Data
    };

    operator squi::Child() const;
};