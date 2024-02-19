#pragma once

#include <vector>
#include "../component.hpp"

struct ComponentStore {
    static const std::vector<std::reference_wrapper<const Component>> components;
};