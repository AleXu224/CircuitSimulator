#include "card.hpp"
#include "box.hpp"

using namespace squi;

Card::operator squi::Child() const {
	return Box{
		.widget{
			.height = Size::Shrink,
			.padding = 4.f,
		},
		.color = Color(1.f, 1.f, 1.f, 0.0512f),
		.borderColor = Color(0.f, 0.f, 0.f, 0.1f),
		.borderWidth{1.f},
		.borderRadius{8.f},
		.borderPosition = squi::Box::BorderPosition::outset,
		.child = child,
	};
}

