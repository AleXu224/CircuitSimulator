#pragma once
#include "widget.hpp"

struct ResultsItem {
	// Args
	std::vector<std::string_view> items{};
	static inline uint32_t counter = 0;
	std::function<void()> onClick;

	operator squi::Child() const;
};