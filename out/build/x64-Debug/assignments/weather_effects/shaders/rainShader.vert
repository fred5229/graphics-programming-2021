#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform vec3 offsets;
uniform float boxSize;
uniform vec3 cameraPosition;
uniform vec3 forwardOffset;

void main()
{
	vec3 position = mod(pos + offsets, boxSize);

	position += cameraPosition + forwardOffset - boxSize/2;

	gl_Position = model * vec4(pos, 0);
}