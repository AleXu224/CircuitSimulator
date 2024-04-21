#pragma once

#include "string"
#include <vector>

using PropertyIndex = int64_t;

struct PropertyData {
    std::string name;
    std::string suffix;
    bool displayable = true;
    PropertyIndex type;
};

struct PropertySet {
	std::string name{};
    std::vector<PropertyData> properties{};
};
