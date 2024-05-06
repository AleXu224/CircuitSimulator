#pragma once

#include "../property/propertyUtils.hpp"

static const Component inductor{
	.name = "Inductor",
	.prefix = "L",
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
	.texturePath = R"(./assets/inductor.png)",
	.textureThumbPath = R"(./assets/inductorThumb.png)",
	.properties{
		PropertySet{
			.properties{
				PropertyData{
					.name{"Inductance"},
					.suffix{"H"},
					.type = PropertyIndexOf<NumberProperty>,
				},
			},
		},
	},
};