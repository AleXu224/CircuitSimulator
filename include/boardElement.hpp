#include "widget.hpp"
#include <cstdint>

struct BoardElement {
	struct Dimensions {
		uint32_t width = 1;
		uint32_t height = 1;
	};

	Dimensions dimensions{};
	squi::Child child;
};