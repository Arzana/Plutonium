#version 460 core

layout (binding = 1) uniform sampler2D Texture;

layout (location = 0) in vec2 Uv;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(Texture, Uv);
}