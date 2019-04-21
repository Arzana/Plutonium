#version 460 core

layout (binding = 1, set = 0) uniform ObjectSpecific
{
	vec4 Color;
};

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = Color;
}