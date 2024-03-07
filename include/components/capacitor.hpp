#pragma once

#include "../component.hpp"

static const Component capacitor{
	.name = "Capacitor",
	.width = 2,
	.height = 3,
	.nodes{
		Coords{
			.x = 1,
			.y = 0,
		},
		Coords{
			.x = 1,
			.y = 3,
		},
	},
	.texturePath = R"(..\assets\capacitor.png)",
	.textureThumbPath = R"(..\assets\capacitorThumb.png)",
};