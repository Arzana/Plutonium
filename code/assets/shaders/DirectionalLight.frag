#version 460 core
#extension GL_KHR_vulkan_glsl : enable

const float PI = 3.141592653589793;
const float EPSLION = 0.00001f;

// Defines camera parameters, needed for decoding and light calculations.
layout (binding = 1) uniform Camera
{
	mat4 IProjection;
	mat4 IView;
	vec3 CamPos;
};

layout (input_attachment_index = 1, set = 1, binding = 0) uniform subpassInput GBufferDiffuseA2;	// Stores the Diffuse color and Roughness^2.
layout (input_attachment_index = 2, set = 1, binding = 1) uniform subpassInput GBufferSpecular;		// Stores the Specular color and power.
layout (input_attachment_index = 3, set = 1, binding = 2) uniform subpassInput GBufferNormal;		// Stores the normal in spherical world coorinates.
layout (input_attachment_index = 5, set = 1, binding = 3) uniform subpassInput GBufferDepth;		// Stores the deth of the scene.

layout (set = 2, binding = 0) uniform Light
{
	vec3 Direction;
	vec3 Radiance;
};

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 L0;

// Schlick
vec3 fresnel(in float ndh, in vec3 f0)
{
	return f0 + (1.0f - f0) * pow(1.0f - ndh, 5);
}

// Cook-Torrance
float occlusion(float ndl, float ndv, float ndh, float vdh)
{
	const float a = (2.0f * ndh * ndv) / vdh;
	const float b = (2.0f * ndh * ndl) / vdh;
	return min(1.0f, min(a, b));
}

// GTR
float microfacet(float ndh, float a2, float power)
{
	const float c2 = ndh * ndh;
	const float s = sqrt(1.0f - c2);
	const float s2 = s * s;

	return (a2 / PI) / pow(a2 * c2 + s2, power);
}

// Decodes the normal from optimzed spherical to a world normal.
vec3 DecodeNormal()
{
	vec2 raw = subpassLoad(GBufferNormal).xy;
	float st = sqrt(1.0f - raw.x * raw.x);
	float sp = sin(raw.y);
	float cp = cos(raw.y);
	return vec3(st * cp, st * sp, raw.x);
}

// Decodes the position from linear depth buffer.
vec3 DecodePosition()
{
	float ld = subpassLoad(GBufferDepth).r;
	vec4 ndc = vec4(Uv * 2.0f - 1.0f, ld, 1.0f);
	vec4 eye = IProjection * ndc;
	eye /= eye.w;
	return (IView * eye).xyz;
}

float mdot(in vec3 a, in vec3 b)
{
	return max(0.0f, dot(a, b));
}

void main()
{
	// Get all of the values out of our G-Buffer.
	const vec4 diffA2 = subpassLoad(GBufferDiffuseA2);
	const vec4 spec = subpassLoad(GBufferSpecular);
	const vec3 position = DecodePosition();
	const vec3 normal = DecodeNormal();

	// Pre-calculate often used values values.
	const vec3 v = normalize(CamPos - position);
	const vec3 h = normalize(v + Direction);
	const float ndl = mdot(normal, Direction);
	const float ndv = mdot(normal, v);
	const float ndh = mdot(normal, h);
	const float vdh = mdot(v, h);

	// Specular
	const vec3 f = fresnel(ndh, spec.xyz);
	const float g = occlusion(ndl, ndv, ndh, vdh);
	const float d = microfacet(ndh, diffA2.w, spec.w);

	// Composition
	const vec3 fd = (1.0f - f) * (diffA2.rgb / PI);
	const vec3 fs = (f * g * d) / (4.0f * ndl * ndv + EPSLION);
	L0 = vec4((fd + fs) * Radiance, 1.0f);
}