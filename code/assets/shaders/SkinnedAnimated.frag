#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0, set = 1) uniform sampler2D Albedo;

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(Albedo, Uv);
}