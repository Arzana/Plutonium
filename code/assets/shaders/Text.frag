#version 460 core

layout (binding = 1, set = 0) uniform StringSpecific
{
	vec4 Color;
};

layout (binding = 0, set = 1) uniform sampler2D Atlas;

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(Atlas, Uv).r * Color;
}