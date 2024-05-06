#pragma once

#include "../property/propertyUtils.hpp"

static const Component resistor{
	.name = "Resistor",
	.prefix = "R",
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
	.texturePath = R"(./assets/resistor.png)",
	.textureThumbPath = R"(./assets/resistorThumb.png)",
	.properties{
		PropertySet{
			.properties{
				PropertyData{
					.name{"Resistance"},
					.suffix{"Ω"},
					.type = PropertyIndexOf<NumberProperty>,
				}
			},
		},
	},
};