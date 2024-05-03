#version 450
layout(binding = 0) uniform Ubo {
	mat4 view;
}
ubo;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inSize;
layout(location = 2) in vec2 inPos;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec2 inLineStart;
layout(location = 5) in vec2 inLineEnd;
layout(location = 6) in float inLineWidth;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragSize;
layout(location = 2) out vec2 fragPos;
layout(location = 3) out vec2 fragLineStart;
layout(location = 4) out vec2 fragLineEnd;
layout(location = 5) out float fragLineWidth;

void main() {
	vec2 pos = inPos + inUv * inSize;
	gl_Position = ubo.view * vec4(pos, 1.0, 1.0);
	fragColor = inColor;
	fragSize = inSize;
	fragPos = pos;
	fragLineStart = inLineStart;
	fragLineEnd = inLineEnd;
	fragLineWidth = inLineWidth;
}