#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (constant_id = 0) const bool RunFxaa = false;

layout (binding = 2) uniform Camera
{
	float Exposure;
	float Brightness;
	float Contrast;
	float Saturation;
};

layout (set = 1, binding = 0) uniform sampler2D HdrBuffer;

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 FragColor;

float rgbToLuma(in vec3 clr)
{
	return dot(clr, vec3(0.2125f, 0.7154f, 0.0721f));
}

vec3 fxaa(in float luma, in float nw, in float ne, in float sw, in float se, in float lumaMin, in float lumaMax)
{
	const vec2 texelStep = 1.0f / textureSize(HdrBuffer, 0);

	// Calculate the sample direction.
	vec2 sampleDir = vec2(-((-nw + ne) - (sw + se)), (nw + sw) - (ne + se));
	const float reduce = max(1.0f / 128.0f, (nw + ne + sw + se) * 0.25f * 0.125f);
	const float minSampleDir = 1.0f / (min(abs(sampleDir.x), abs(sampleDir.y)) + reduce);
	sampleDir = clamp(sampleDir * minSampleDir, vec2(-8.0f), vec2(8.0f)) * texelStep;

	// Get the inner samples on the tab.
	const vec3 ihdrSampleIn = textureLod(HdrBuffer, Uv + sampleDir * (1.0f / 3.0f - 0.5f), 0.0f).rgb;
	const vec3 hdrSampleIn = textureLod(HdrBuffer, Uv + sampleDir * (2.0f / 3.0f - 0.5f), 0.0f).rgb;
	const vec3 hdr2Tab = (hdrSampleIn + ihdrSampleIn) * 0.5f;

	// Get the outer samples on the tab.
	const vec3 ihdrSampleOut = textureLod(HdrBuffer, Uv + sampleDir * -0.5f, 0.0f).rgb;
	const vec3 hdrSampleOut = textureLod(HdrBuffer, Uv + sampleDir * 0.5f, 0.0f).rgb;
	const vec3 hdr4Tab = (hdrSampleOut + ihdrSampleOut) * 0.5f;

	// Check if luma is within the specified range, if so return the two tab, otherwise; return the four tab.
	const float luma4Tab = rgbToLuma(hdr4Tab);
	const float lambda = float(luma4Tab < lumaMin || luma4Tab > lumaMax);
	return hdr2Tab * lambda + hdr4Tab * (1.0f - lambda);
}

void main()
{
	vec3 raw = textureLod(HdrBuffer, Uv, 0.0f).rgb;

	if (RunFxaa)
	{
		// Sample the neightbouring texels and convert them to luma.
		const float luma = rgbToLuma(raw);
		const float lumaNW = rgbToLuma(textureLodOffset(HdrBuffer, Uv, 0.0f, ivec2(-1, -1)).rgb);
		const float lumaNE = rgbToLuma(textureLodOffset(HdrBuffer, Uv, 0.0f, ivec2(1, -1)).rgb);
		const float lumaSW = rgbToLuma(textureLodOffset(HdrBuffer, Uv, 0.0f, ivec2(-1, 1)).rgb);
		const float lumaSE = rgbToLuma(textureLodOffset(HdrBuffer, Uv, 0.0f, ivec2(1, 1)).rgb);

		// Get the luma range.
		const float lumaMin = min(luma, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
		const float lumaMax = max(luma, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

		// Only apply FXAA if the contrast is higher than the threshold.
		if (lumaMax - lumaMin >= lumaMax * 0.5f)
		{
			raw = fxaa(luma, lumaNW, lumaNE, lumaSW, lumaSE, lumaMin, lumaMax);
		}
	}

	// Apply brightness, saturation and contrast.
	const vec3 clrBright = raw * Brightness;
	const vec3 clr = mix(vec3(0.5f), mix(rgbToLuma(clrBright).xxx, clrBright, Saturation), Contrast);

	// Tonemap using Reinhard
	FragColor = vec4(1.0f - exp(-clr * Exposure), 1.0f);
}