#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec2 fragTexUv;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;


float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main() {
	vec3 msd = texture(tex, fragTexUv).rgb;

	// vec3 msd = texture(msdf, texCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b) - 0.5;
	// FIXME: implement screenPxRange()
	vec2 grad_dist = vec2(dFdx(sd), dFdy(sd));
	float afwidth = 0.7071067f * length(grad_dist);
	float coverage = smoothstep(afwidth, -afwidth, sd);

	// vec2 grad_dist = normalize(vec2(dFdx(sd), dFdy(sd)));
	// vec2 Jdx = dFdx(fragUv);
	// vec2 Jdy = dFdy(fragUv);
	// vec2 grad = vec2(grad_dist.x * Jdx.x + grad_dist.y*Jdy.y, grad_dist.x * Jdx.y + grad_dist.y*Jdy.y);
	// float afwidth = 0.7071*length(grad);
	// float coverage = smoothstep(afwidth, -afwidth, sd);


	outColor = mix(vec4(fragColor.xyz * fragColor.a, fragColor.a), vec4(0.f), coverage);
}