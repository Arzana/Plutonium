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

layout (input_attachment_index = 4, set = 2, binding = 5) uniform subpassInput HdrBuffer;

layout (location = 0) in vec2 Uv;					// We don't use the Uv, but we need to pass it for the shaders to properly connect.
layout (location = 0) out vec4 FragColor;

vec3 choose(vec3 hdr, vec3 ldr)
{
	const float lambda = 1.0f - HdrSwapchain;
	return hdr * HdrSwapchain + lambda * ldr;
}

void main()
{
	const vec3 raw = subpassLoad(HdrBuffer).rgb;	// Get the HDR value.
	const vec3 hdr = raw * Contrast + Brightness;	// Scale the image to out desired contrast and brighness.
	const vec3 ldr = 1.0f - exp(-hdr * Exposure);	// Get the LDR value (reinhard) that we might need if we have a non-native HDR display.
	FragColor = vec4(choose(hdr, ldr), 1.0f);		// Either use the scaled HDR value or the tone mapped LDR value, depends on the display.
}