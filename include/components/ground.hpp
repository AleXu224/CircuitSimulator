#pragma once

#include "../component.hpp"

static const Component ground{
	.name = "Ground",
	.width = 2,
	.height = 2,
	.type = ElementType::Ground,
	.nodes{
		Coords{
			.x = 1,
			.y = 0,
		},
	},
	.texturePath = R"(./assets/ground.png)",
	.textureThumbPath = R"(./assets/ground.png)",
};