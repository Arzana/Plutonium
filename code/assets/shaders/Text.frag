#version 460 core

layout (binding = 1) uniform Parameters
{
	vec4 Color;
};

layout (binding = 2) uniform sampler2D Atlas;

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(Atlas, Uv) * Color;
}