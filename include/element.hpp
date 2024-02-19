#pragma once

#include "coords.hpp"

struct Element {
	const Coords size{1, 1};
	Coords pos{0, 0};
	uint32_t rotation = 0;

	Coords getSize() const {
		if (rotation % 2 == 0) {
			return size;
		}
		return {size.y, size.x};
	}
};