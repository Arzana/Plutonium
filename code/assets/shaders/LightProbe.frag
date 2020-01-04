#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (set = 1, binding = 0) uniform sampler2D Diffuse;

layout (location = 0) in vec2 FragUv;

layout (location = 0) out vec4 Color;

void main()
{
	const vec4 diff = texture(Diffuse, FragUv);
	if (diff.a < 0.5f) discard;
	Color = diff;
}