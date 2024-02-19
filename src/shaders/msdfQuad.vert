#version 450
layout(binding = 0) uniform Ubo {
	mat4 view;
}
ubo;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inSize;
layout(location = 2) in vec2 inPos;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec2 inTexUv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec2 fragTexUv;

void main() {
	vec2 pos = inPos + inUv * inSize;
	gl_Position = ubo.view * vec4(pos, 1.0, 1.0);
    fragColor = inColor;
	fragUv = inUv;
	fragTexUv = inTexUv;
}