#pragma once

#include "../property/propertyUtils.hpp"

static const Component currentSource{
	.name = "Current Source",
	.prefix = "I",
	.width = 2,
	.height = 4,
	.nodes{
		Coords{
			.x = 1,
			.y = 4,
		},
		Coords{
			.x = 1,
			.y = 0,
		},
	},
	.texturePath = R"(.\assets\currentSourceEU.png)",
	.textureThumbPath = R"(.\assets\currentSourceEUThumb.png)",
	.uvTopLeft{0.25, 0},
	.uvBottomRight{0.75, 1},
	.properties{
		PropertySet{
			.name = "D.C. Mode",
			.properties{
				PropertyData{
					.name{"Current"},
					.suffix{"A"},
					.type = PropertyIndexOf<NumberProperty>,
				},
			},
		},
		PropertySet{
			.name = "A.C. Mode",
			.properties{
				PropertyData{
					.name{"Amplitude"},
					.suffix{"A"},
					.type = PropertyIndexOf<NumberProperty>,
				},
				PropertyData{
					.name{"Phase"},
					.suffix{"°"},
					.type = PropertyIndexOf<NumberProperty>,
				},
			},
		},
	},
};