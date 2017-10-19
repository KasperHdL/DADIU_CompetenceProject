#version 410
uniform mat4 matrix;

layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec4 v2UVcoordsIn;
layout(location = 3)in vec4 color;

out vec2 v2UVcoords;

void main()
{
	v2UVcoords = v2UVcoordsIn;
	gl_Position = matrix * vec4(position,1);
}