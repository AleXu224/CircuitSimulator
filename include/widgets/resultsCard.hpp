#pragma once
#include "widget.hpp"

struct ResultsCard {
	// Args
	squi::Widget::Args widget{};
	std::string_view title{};
	std::vector<std::string_view> columns{};
	squi::Children children{};

	operator squi::Child() const;
};