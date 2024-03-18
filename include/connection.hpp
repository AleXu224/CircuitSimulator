#pragma once

#include "element.hpp"
#include "widget.hpp"


struct Connection {
	size_t nodeIndex;
	std::reference_wrapper<Element> element;
	squi::ChildRef widget;
};