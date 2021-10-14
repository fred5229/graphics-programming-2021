#version 330 core
layout (location = 0) in vec3 pos;

out float lenColorScale;

uniform mat4 model;
uniform mat4 prevModel;
uniform vec3 offsets;
uniform float boxSize;
uniform vec3 cameraPosition;
uniform vec3 forwardOffset;
uniform float heightScale;
uniform vec3 g_vVelocity;

void main()
{
	// calculate motion blur as described in the paper
	vec3 position = mod(pos + offsets, boxSize);

	position += cameraPosition + forwardOffset - boxSize/2;

	vec4 worldPos = vec4(position, 1);
	vec4 worldPosPrev = vec4(position, 1) + vec4(g_vVelocity, 1.0) * heightScale;

	worldPosPrev.w = 1.0f;

	vec4 bottom = model * worldPos;
	vec4 topPrev = prevModel * worldPosPrev;
	vec4 top = model * worldPosPrev;

	vec4 finalPos = mix(topPrev, bottom, gl_VertexID % 2);

	// calculate alpha blending as described in the paper
	vec2 dir = (top.xy/top.w) - (bottom.xy/bottom.w);
	vec2 dirPrev = (topPrev.xy/topPrev.w) - (bottom.xy/bottom.w);

	float len = length(dir);
	float lenPrev = length(dirPrev);

	lenColorScale = clamp(len/lenPrev, 0.0, 1.0);

	gl_Position = finalPos;
}