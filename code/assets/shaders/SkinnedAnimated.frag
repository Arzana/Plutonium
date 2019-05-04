#version 460 core
#extension GL_KHR_vulkan_glsl : enable

const float PI = 3.141592653589793;

layout (binding = 0, set = 1) uniform sampler2D Albedo;

layout (binding = 1, set = 0) uniform Globals
{
	vec3 CamPos;
};

layout (location = 0) in vec2 Uv;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Position;

layout (location = 0) out vec4 FragColor;

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

	return a2 / (PI * pow(ndh * ndh * (a2 - 1.0f) + 1.0f, 2.0f));
}

void main()
{
	// Constants. 
	const vec3 f0 = vec3(0.0f);						// Specular = [0, 0, 0]
	const float a = 0.0f;							// Glossiness = 1,		a = (1 - glossiness)^2
	const vec3 v = normalize(CamPos - Position);	// View direction
	const vec3 l = -vec3(0.7f, 0.7f, 0.0f);			// Light direction
	const vec3 h = normalize(l + v);				// Halfway vector

	// BRDF.
	const float d = TrowbridgeReitz(Normal, h, a);	// Microfacet
	const float g = GGX(Normal, l, v, a);			// Geomerty occlusion
	const vec3 f = FresnelSchlick(f0, v, h);		// Fresnel
	
	// Diffuse and specular factors.
	const vec3 fs = f;
	const vec3 fd = 1.0f - fs;

	// Actual diffuse and specular, with ambient term.
	const vec3 diffuse = texture(Albedo, Uv).rgb * (1.0f - max(f0.r, max(f0.g, f0.b)));
	const vec3 specular = (f * g * d) / (4.0f * dot(Normal, l) * dot(Normal, v));
	const vec3 ambient = 0.03f * diffuse;

	// Output color.
	FragColor = vec4(ambient + fd * diffuse + specular, 1.0f);
}