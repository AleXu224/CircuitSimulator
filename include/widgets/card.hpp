#pragma once
#include "widget.hpp"

struct Card {
	// Args
	squi::Child child{};

	operator squi::Child() const;
};