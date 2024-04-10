#include "coords.hpp"
#include "widget.hpp"

struct NodeIndexDisplay {
    // Args
    squi::Widget::Args widget{};
    uint32_t nodeIndex{};
    Coords pos;

    struct Storage {
        // Data
    };

    operator squi::Child() const;
};