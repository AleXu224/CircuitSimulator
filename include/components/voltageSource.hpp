#pragma once

#include "../property/propertyUtils.hpp"

static const Component voltageSource{
	.name = "Voltage Source",
	.prefix = "V",
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
	.texturePath = R"(.\assets\voltageSourceEU.png)",
	.textureThumbPath = R"(.\assets\voltageSourceEUThumb.png)",
	.uvTopLeft{0.25, 0},
	.uvBottomRight{0.75, 1},
	.properties{
		PropertySet{
			.name = "D.C. Mode",
			.properties{
				PropertyData{
					.name{"Voltage"},
					.suffix{"V"},
					.type = PropertyIndexOf<NumberProperty>,
				},
			},
		},
		PropertySet{
			.name = "A.C. Mode",
			.properties{
				PropertyData{
					.name{"Amplitude"},
					.suffix{"V"},
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