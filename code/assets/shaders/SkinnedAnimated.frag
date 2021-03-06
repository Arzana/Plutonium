#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (early_fragment_tests) in;

const float PI = 3.141592653589793;

layout (binding = 1, set = 0) uniform Globals
{
	vec3 CamPos;
};

layout (binding = 0, set = 1) uniform sampler2D Diffuse;

layout (binding = 1, set = 1) uniform Material
{
	vec3 F0;
	vec3 DiffuseFactor;
	float Glossiness;
};

layout (location = 0) in vec2 Uv;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Position;

layout (location = 0) out vec4 L0;

vec3 FresnelSchlick(in vec3 f0, in vec3 v, in vec3 h)
{
	return f0 + (1.0f - f0) * pow(1.0f - dot(v, h), 5);
}

float GGX(in vec3 n, in vec3 l, in vec3 v, in float a)
{
	const float a2 = a * a;
	const float ia2 = 1.0f - a2;
	const float ndl = dot(n, l);
	const float ndv = dot(n, v);

	return 0.5f / (ndl * sqrt(ndv * ndv * ia2 + a2) + ndv * sqrt(ndl * ndl * ia2 + a2));
}

float TrowbridgeReitz(in vec3 n, in vec3 h, in float a)
{
	const float a2 = a * a;
	const float ndh = dot(n, h);
	const float f = (ndh * a2 - ndh) * ndh + 1.0f;
	return a2 / (PI * f * f);
}

vec3 diffuse()
{
	const vec3 base = texture(Diffuse, Uv).rgb * DiffuseFactor;
	const float iSpec = 1.0f - max(max(F0.r, F0.b), F0.g);
	return base * iSpec / PI;
}

void main()
{
	// Constants. 
	const float a = pow(1.0f - Glossiness, 2);			// Glossiness
	const vec3 v = normalize(CamPos - Position);		// View direction
	const vec3 l = normalize(-vec3(0.7f, 0.7f, 0.0f));	// Light direction
	const vec3 h = normalize(l + v);					// Halfway vector

	// BRDF (Cook-Torrance).
	const float d = TrowbridgeReitz(Normal, h, a);		// Microfacet
	const float g = GGX(Normal, l, v, a);				// Geomerty occlusion
	const vec3 f = FresnelSchlick(F0, v, h);			// Fresnel
	
	// Diffuse and specular factors.
	const vec3 fs = f * g * d;
	const vec3 fd = (1.0f - f) * diffuse();

	// Output color.
	L0 = vec4(max(0.0f, dot(Normal, l)) * (fd + fs), 1.0f);
}