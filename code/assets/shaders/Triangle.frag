#version 460 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 Uv;
layout (location = 0) uniform vec4 Diffuse;

void main()
{
	FragColor = vec4(0.0f, 0.4f, 1.0f, 1.0f);
}