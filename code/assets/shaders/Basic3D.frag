#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (early_fragment_tests) in;

const float PI = 3.141592653589793;
const float EPSLION = 0.00001f;

layout (binding = 1, set = 0) uniform Globals
{
	vec3 CamPos;
};

//layout (binding = 0, set = 1) uniform sampler2D Diffuse;

// We could store the glossiness in the diffuse factor to save 4 bytes due to allignment.
layout (binding = 1, set = 1) uniform Material
{
	vec3 F0;
	vec3 DiffuseFactor;
	float Roughness;
	float SpecularPower;
};

layout (location = 0) in vec2 Uv;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Position;

layout (location = 0) out vec4 L0;

// Oren-Nayar
vec3 diffuse(float ndl, float ndv, float a2)
{
	const vec3 cdiff = DiffuseFactor / PI;
	const vec3 lambert = cdiff * ndl;

	const float theta = acos(ndl);
	const float phi = acos(ndv);

	const float A = 1.0f - 0.5f * (a2 / (a2 + 0.33f));
	const float B = 0.45f * (a2 / (a2 + 0.09f));

	const float alpha = max(theta, phi);
	const float beta = min(theta, phi);

	return lambert * (A + (B * max(0.0f, cos(ndv - ndl)) * sin(alpha) * tan(beta)));
}

// Schlick
vec3 fresnel(float ndl)
{
	return F0 + (1.0f - F0) * pow(1.0f - max(0.0f, ndl), 5);
}

// Cook-Torrance
float occlusion(float ndl, float ndv, float ndh, float vdh)
{
	const float a = (2 * ndh * ndv) / vdh;
	const float b = (2 * ndh * ndl) / vdh;
	return min(1, min(a, b));
}

// GTR
float microfacet(float ndh, float a2)
{
	const float c2 = ndh * ndh;
	const float s = sqrt(1.0f - c2);
	const float s2 = s * s;

	return (a2 / PI) / pow(a2 * c2 + s2, SpecularPower);
}

void main()
{
	// Constants. 
	const vec3 v = normalize(CamPos - Position);
	const vec3 l = normalize(-vec3(0.7f, 0.7f, 0.0f));
	const vec3 h = normalize(v + l);

	// Intermediates
	const float a2 = Roughness * Roughness + EPSLION;
	const float ndl = dot(Normal, l);
	const float ndv = dot(Normal, v);
	const float ndh = dot(Normal, h);
	const float vdh = dot(v, h);

	// Specular
	const vec3 f = fresnel(ndl);
	const float g = occlusion(ndl, ndv, ndh, vdh);
	const float d = microfacet(ndh, a2);

	// Composition
	const vec3 fd = (1.0f - f) * diffuse(ndl, ndv, a2);
	const vec3 fs = (f * g * d) / (4.0f * ndl * ndv + EPSLION);

	L0 = vec4(fd + fs, 1.0f);
}