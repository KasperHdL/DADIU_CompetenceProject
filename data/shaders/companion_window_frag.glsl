#version 410 core

uniform sampler2D mytexture;

noperspective in vec2 v2UV;

out vec4 outputColor;

void main()
{
    outputColor = texture(mytexture, vec2(v2UV.x, 1.0f - v2UV.y));
}