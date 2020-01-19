#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 2) uniform Globals
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
	const vec3 hdr = texture(HdrBuffer, Uv).rgb;		// Get the HDR value.
	const vec3 scaled = hdr * Contrast + Brightness;	// Scale the image to out desired contrast and brighness.
	L0 = vec4(1.0f - exp(-hdr * Exposure), 1.0f);		// Convert the HDR value to LDR (reinhard).
}