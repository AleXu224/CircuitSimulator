#pragma once

#include "functional"
#include <algorithm>
#include <ostream>

struct Coords {
	int x = 0;
	int y = 0;

	bool operator==(const Coords &other) const {
		return x == other.x && y == other.y;
	}

	[[nodiscard]] Coords operator+(const Coords &other) const {
		return {x + other.x, y + other.y};
	}
	[[nodiscard]] Coords operator-(const Coords &other) const {
		return {x - other.x, y - other.y};
	}

	static Coords max(const Coords &left, const Coords &right) {
		return {
			std::max(left.x, right.x),
			std::max(left.y, right.y),
		};
	}
	static Coords min(const Coords &left, const Coords &right) {
		return {
			std::min(left.x, right.x),
			std::min(left.y, right.y),
		};
	}

	friend std::ostream &operator<<(std::ostream &os, const Coords &rhs) {

		os << "x: " << rhs.x
		   << " y: " << rhs.y;

		return os;
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