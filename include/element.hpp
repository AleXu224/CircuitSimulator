#pragma once

#include "coords.hpp"
#include <vector>
#include "component.hpp"

enum class ElementType {
	Line,
	Other,
};

struct Element {
	uint32_t id = idCounter++;
	Coords size{1, 1};
	Coords pos{0, 0};
	uint32_t rotation = 0;
	std::vector<Coords> nodes{};
	std::reference_wrapper<const Component> component;
	ElementType type = ElementType::Other;

	bool operator==(const Element& other) const {
		return this->id == other.id;
	}

	static inline uint32_t idCounter = 0;
};