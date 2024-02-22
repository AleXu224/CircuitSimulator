#pragma once

#include "../component.hpp"

static const Component resistor{
	.name = "Resistor",
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
	.texturePath = R"(C:\Users\Squizell\Desktop\CircuitSimulatorAssets\resistor.png)",
};