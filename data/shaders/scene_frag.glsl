#version 410 core

uniform vec4 color;

in vec2 v2UVcoords;
out vec4 outputColor;

void main()
{
   outputColor = color;
}