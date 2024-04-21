#pragma once

#include "component.hpp"
#include "coords.hpp"
#include <vector>
#include "property/propertyUtils.hpp"

using ElementId = uint32_t;

struct Element {
	ElementId id = idCounter++;
	Coords size{1, 1};
	Coords pos{0, 0};
	uint32_t rotation = 0;
	std::vector<Coords> nodes{};
	std::reference_wrapper<const Component> component;
	mutable size_t propertySetIndex = 0;
	mutable std::vector<PropertyVariant> propertiesValues{};

	bool operator==(const Element &other) const {
		return this->id == other.id;
	}

	static inline uint32_t idCounter = 0;
};