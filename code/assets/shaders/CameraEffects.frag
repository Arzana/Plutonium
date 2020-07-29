#version 460 core
#extension GL_KHR_vulkan_glsl : enable

const vec3 RGB2LUM = vec3(0.2125f, 0.7154f, 0.0721f); 

layout (binding = 2) uniform Camera
{
	float Exposure;
	float Brightness;
	float Contrast;
	float Saturation;
};

layout (input_attachment_index = 4, set = 1, binding = 0) uniform subpassInput HdrBuffer;

layout (location = 0) in vec2 Uv;					// We don't use the Uv, but we need to pass it for the shaders to properly connect.
layout (location = 0) out vec4 FragColor;

void main()
{
	// Calculate the HDR value after the camera effects.
	const vec4 raw = subpassLoad(HdrBuffer).rgba;

	const vec3 clrBright = raw.rgb * Brightness;
	const vec3 clr = mix(vec3(0.5f), mix(dot(clrBright, RGB2LUM).xxx, clrBright, Saturation), Contrast);

	// Tonemap using Reinhard
	FragColor = vec4(1.0f - exp(-clr * Exposure), 1.0f);
}