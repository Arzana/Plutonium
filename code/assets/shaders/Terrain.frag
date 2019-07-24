#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (early_fragment_tests) in;

const float PI = 3.141592653589793;

layout (binding = 1, set = 0) uniform Globals
{
	vec3 CamPos;
};

// We could store the glossiness in the diffuse factor to save 4 bytes due to allignment.
layout (binding = 1, set = 1) uniform Material
{
	vec3 F0;
	vec3 DiffuseFactor;
	float Roughness;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in float Height;

layout (location = 0) out vec4 L0;

float AlphaRoughness2()
{
	return Roughness * Roughness * Roughness * Roughness;
}

// Schlick
vec3 Fresnel(in vec3 v, in vec3 h)
{
	return F0 + (1.0f - F0) * pow(1.0f - dot(v, h), 5);
}

// Smith Joint GGX
float GeometricOcclusion(in vec3 l, in vec3 v)
{
	const float a2 = AlphaRoughness2();
	const float ia2 = 1.0f - a2;

	const float ndl = dot(Normal, l);
	const float ndv = dot(Normal, v);

	const float shadow = ndl * sqrt(ndv * ndv * ia2 + a2);
	const float mask = ndv * sqrt(ndl * ndl * ia2 + a2);

	return 0.5f / (shadow + mask);
}

// Throwbridge-Reitz GGX
float Microfacet(in vec3 h)
{
	const float a2 = AlphaRoughness2();
	const float ndh = dot(Normal, h);
	const float f = ndh * ndh * (a2 - 1.0f) + 1.0f;
	return a2 / (PI * f * f);
}

// Interpolates between a and b, the amount is determinded by an inverse lerp between c, d, and v.
vec3 interp(vec3 a, vec3 b, float c, float d, float v)
{
	return mix(a, b, (v - c) / (d - c));
}

vec3 diffuse()
{
	vec3 snow = vec3(1.0f);
	vec3 rock = vec3(0.35f, 0.27f, 0.23f);
	vec3 grass = vec3(0.33f, 0.59f, 0.09f);
	vec3 water = vec3(0.26f, 0.45f, 0.81f);

	if (Height > 0.9f) return snow;
	else if (Height > 0.7f) return interp(rock, snow, 0.7f, 0.9f, Height);
	else if (Height > 0.4f) return interp(grass, rock, 0.4f, 0.7f, Height);
	return interp(water, grass, 0.0f, 0.7f, Height);	// Water needs to be more defined so we put the interpolation point higher to have less grass.
}

void main()
{
	// Constants. 
	const vec3 v = normalize(CamPos - Position);		// View direction
	const vec3 l = normalize(-vec3(0.7f, 0.7f, 0.0f));	// Light direction
	const vec3 h = normalize(l + v);					// Halfway vector

	// BRDF (Cook-Torrance).
	const float d = Microfacet(h);
	const float g = GeometricOcclusion(l, v);
	const vec3 f = Fresnel(v, h);
	const float vis = g / (4.0f * dot(Normal, l) * dot(Normal, v));
	
	// Diffuse and specular factors.
	const vec3 fs = f * vis * d;
	const vec3 fd = (1.0f - f) * diffuse();

	// Output color.
	L0 = vec4(max(0.0f, dot(Normal, l)) * (fd + fs), 1.0f);
}