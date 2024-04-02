#pragma once

#include "element.hpp"
#include "widget.hpp"

struct PropertyEditor {
    // Args
    squi::Widget::Args widget{};
    const Element& element;

    struct Storage {
        // Data
    };

    operator squi::Child() const;
};