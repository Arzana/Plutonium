#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (push_constant, std140) uniform Display
{
	layout (offset = 4) float HdrSwapchain;
};

layout (binding = 2) uniform Camera
{
	float Exposure;
	float Brightness;
	float Contrast;
};

layout (set = 4, binding = 0) uniform sampler2D HdrBuffer;

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 L0;

void main()
{
	const vec3 raw = texture(HdrBuffer, Uv).rgb;							// Get the HDR value.
	const vec3 hdr = raw * Contrast + Brightness;							// Scale the image to out desired contrast and brighness.
	const vec3 ldr = 1.0f - exp(-hdr * Exposure);							// Get the LDR value (reinhard) that we might need if we have a non-native HDR display.
	L0 = vec4((hdr * HdrSwapchain) + (ldr * (1.0f - HdrSwapchain)), 1.0f);	// Either use the scaled HDR value or the tone mapped LDR value, depends on the display.
}