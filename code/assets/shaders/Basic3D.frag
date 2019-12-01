#version 460 core
#extension GL_KHR_vulkan_glsl : enable

const float PI = 3.141592653589793;
const float EPSLION = 0.00001f;

layout (binding = 1, set = 0) uniform Globals
{
	vec3 CamPos;
};

layout (binding = 0, set = 1) uniform sampler2D Diffuse;
layout (binding = 1, set = 1) uniform sampler2D SpecularGlossiness;
layout (binding = 2, set = 1) uniform sampler2D Normal;

// We could store the glossiness in the diffuse factor to save 4 bytes due to allignment.
layout (binding = 3, set = 1) uniform Material
{
	vec3 F0;
	vec3 DiffuseFactor;
	float Roughness;
	float SpecularPower;
};

layout (location = 0) in vec2 Uv;
layout (location = 1) in vec3 Position;
layout (location = 2) in mat3 TBN;

layout (location = 0) out vec4 L0;

// Lambert
vec3 diffuse(vec4 diff, float ndl)
{
	const vec3 cdiff = diff.rgb / PI;
	return cdiff * ndl;
}

float ilerp(vec3 a, vec3 b, vec3 x)
{
	return dot(x - a, b - a) / dot(b - a, b - a);
}

// Schlick
vec3 fresnel(float vdh)
{
	const vec3 f0 = texture(SpecularGlossiness, Uv).rgb * F0;
	return f0 + (1.0f - f0) * pow(1.0f - vdh, 5);
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
	// Discard transparent pixels.
	const vec4 diff = texture(Diffuse, Uv) * vec4(DiffuseFactor, 1.0f);
	if (diff.a < 0.5f) discard;

	// Constants. 
	const vec3 v = normalize(CamPos - Position);
	const vec3 l = normalize(vec3(0.7f, 0.7f, 0.0f));
	const vec3 h = normalize(v + l);
	const vec3 radiance = vec3(1.0f, 1.0f, 0.86f);

	// Intermediates
	const float roughness = (1.0f - texture(SpecularGlossiness, Uv).a) * Roughness;
	const float a2 = roughness * roughness + EPSLION;
	const vec3 normal = normalize(TBN * normalize(texture(Normal, Uv).xyz * 2.0f - 1.0f));
	const float ndl = max(0.0f, dot(normal, l));
	const float ndv = max(0.0f, dot(normal, v));
	const float ndh = max(0.0f, dot(normal, h));
	const float vdh = max(0.0f, dot(v, h));

	// Specular
	const vec3 f = fresnel(vdh);
	const float g = occlusion(ndl, ndv, ndh, vdh);
	const float d = microfacet(ndh, a2);

	// Composition
	const vec3 fd = (1.0f - f) * diffuse(diff, ndl);
	const vec3 fs = (f * g * d) / (4.0f * ndl * ndv + EPSLION);

	L0 = vec4((fd + fs) * radiance, 1.0f);
}