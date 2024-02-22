#pragma once

#include "coords.hpp"
#include <vector>

struct Element {
	Coords size{1, 1};
	Coords pos{0, 0};
	uint32_t rotation = 0;
	std::vector<Coords> nodes{};
};