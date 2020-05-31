#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (early_fragment_tests) in;

layout (set = 1, binding = 0) uniform samplerCube Skybox;

layout (location = 0) in vec3 Angle;
layout (location = 0) out vec4 Hdr;

void main()
{
	Hdr = textureLod(Skybox, Angle, 0.0f);
}