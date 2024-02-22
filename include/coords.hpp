#pragma once

#include "functional"

struct Coords {
	int x = 0;
	int y = 0;

	bool operator==(const Coords &other) const {
		return x == other.x && y == other.y;
	}

	[[nodiscard]] Coords operator+(const Coords &other) const {
		return {x + other.x, y + other.y};
	}
};

namespace std {
	template<>
	struct hash<Coords> {
		std::size_t operator()(const Coords &coords) const {
			std::size_t h1 = std::hash<int>{}(coords.x);
			std::size_t h2 = std::hash<int>{}(coords.y);
			return h1 ^ (h2 << 1);
		}
	};
}// namespace std