#version 450
layout(binding = 0) uniform Ubo {
	mat4 view;
}
ubo;

layout(location = 0) in vec2 inSize;
layout(location = 1) in vec2 inPos;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec2 inOffset;

layout(location = 0) out vec2 fragSize;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec2 fragOffset;

void main() {
	vec2 pos = inPos + inUv * inSize;
	gl_Position = ubo.view * vec4(pos, 1.0, 1.0);
	fragSize = inSize;
	fragUv = inUv;
    fragOffset = inOffset;
}