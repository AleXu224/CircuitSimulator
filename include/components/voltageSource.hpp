#pragma once

#include "../component.hpp"

static const Component voltageSource{
	.name = "Voltage Source",
	.width = 2,
	.height = 4,
	.nodes{
		Coords{
			.x = 1,
			.y = 0,
		},
		Coords{
			.x = 1,
			.y = 4,
		},
	},
	.texturePath = R"(.\assets\voltageSource.png)",
	.textureThumbPath = R"(.\assets\voltageSourceThumb.png)",
	.uvTopLeft{0.25, 0},
	.uvBottomRight{0.75, 1},
};