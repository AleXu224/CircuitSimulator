#pragma once

#include "component.hpp"
#include "coords.hpp"
#include "vec2.hpp"


namespace Utils {
	Coords screenToGridRounded(const squi::vec2 &screen, const squi::vec2 &offset, const squi::vec2 &boardPos);
	Coords screenToGridFloored(const squi::vec2 &screen, const squi::vec2 &offset, const squi::vec2 &boardPos);
	Coords componentSizeWithRotation(const Component &component, uint32_t rotation);
	struct RotatedElementData {
		squi::vec2 newTopLeft{};
		squi::vec2 newTopRight{};
		squi::vec2 newBottomRight{};
		squi::vec2 newBottomLeft{};
		std::vector<Coords> newNodes{};
	};
	RotatedElementData rotateElement(uint32_t rotation, const Component &comp);
}// namespace Utils