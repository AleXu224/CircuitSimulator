#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragSize;
layout(location = 2) in vec2 fragPos;
layout(location = 3) in vec2 fragLineStart;
layout(location = 4) in vec2 fragLineEnd;
layout(location = 5) in float fragLineWidth;

layout(location = 0) out vec4 outColor;

void main() {
	// Start of the line
	vec2 p1 = fragLineStart;
	// End of the line
	vec2 p2 = fragLineEnd;
	// Current point
	vec2 p3 = fragPos;
	float width = fragLineWidth;

	float dotProd = dot(p2 - p1, p3 - p1);
	float lenP2 = length(p2 - p1);
	float lenP3 = length(p3 - p1);

	// https://en.wikipedia.org/wiki/Dot_product#Properties
	float cosAlpha = dotProd / (lenP2 * lenP3);
	float lenP4 = cosAlpha * lenP3;

	lenP4 = clamp(lenP4, 0.f, lenP2);
	vec2 p4 = p1 + (p2 - p1) * (lenP4 / lenP2);
	float dist = length(p3 - p4) - (width / 2.f);

	// vec2 grad_dist = vec2(dFdx(dist), dFdy(dist));
	float afwidth = 0.5f /* * length(grad_dist) */;
	float coverage = smoothstep(afwidth, -afwidth, dist);

	outColor = mix(vec4(0.f), vec4(fragColor.xyz * fragColor.a, fragColor.a), coverage);
}