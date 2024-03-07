#pragma once

#include "../component.hpp"

static const Component node{
	.name = "Node",
	.width = 1,
	.height = 1,
	.texturePath = R"(.\assets\node.png)",
	.textureThumbPath = R"(.\assets\resistorThumb.png)",
	.hidden = true,
};