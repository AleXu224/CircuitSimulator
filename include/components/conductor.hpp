#pragma once

#include "../component.hpp"

static const Component conductor{
	.name = "Conductor",
	.prefix = "Conductor",
	.width = 1,
	.height = 1,
	.texturePath = R"(.\assets\conductor.png)",
	.textureThumbPath = R"(.\assets\resistorThumb.png)",
	.hidden = true,
};