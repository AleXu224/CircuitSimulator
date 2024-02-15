#version 450

layout(location = 0) in vec2 fragSize;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec2 fragOffset;

layout(location = 0) out vec4 outColor;

bool multipleOf(vec2 coord, float multiple) {
    return mod(coord.x, multiple) < 1.f || mod(coord.y, multiple) < 1.f;
}

void main() {
	vec4 usedColor;
	vec2 coords = fragUv * fragSize - fragOffset;
    float spaceBetween = 20.f;
    if (multipleOf(coords, 100.f)) usedColor = vec4(0.4f, 0.4f, 0.4f, 1.f);
	else if (multipleOf(coords, spaceBetween)) usedColor = vec4(0.2f, 0.2f, 0.2f, 1.f);
	else usedColor = vec4(0.15, 0.15, 0.15, 1.f);
	outColor = vec4(usedColor.xyz * usedColor.a, usedColor.a);
}