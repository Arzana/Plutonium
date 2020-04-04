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

layout (input_attachment_index = 1, set = 1, binding = 0) uniform subpassInput GBufferDiffuseRough;	// Stores the Diffuse color and Roughness.
layout (input_attachment_index = 2, set = 1, binding = 1) uniform subpassInput GBufferSpecular;		// Stores the Specular color and power.
layout (input_attachment_index = 3, set = 1, binding = 2) uniform subpassInput GBufferNormal;		// Stores the normal in spherical world coorinates.
layout (input_attachment_index = 5, set = 1, binding = 3) uniform subpassInput GBufferDepth;		// Stores the deth of the scene.

layout (set = 2, binding = 0) uniform samplerCube Environment;
layout (set = 2, binding = 1) uniform Light
{
	vec3 Direction;
	vec4 Radiance;
};

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 L0;

float mdot(in vec3 a, in vec3 b)
{
	return max(0.0f, dot(a, b));
}

// F: Schlick
vec3 fresnel(in float cosTheta, in vec3 f0)
{
	return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

// G: Smith Joint GGX (Vis)
float occlusion(in float ndl, in float ndv, in float roughness)
{
	const float r = roughness + 1.0f;
	const float k = r * r / 8.0f;
	const float ggxl = ndl / (ndl * (1.0f - k) + k);
	const float ggxv = ndv / (ndv * (1.0f - k) + k);
	return ggxl * ggxv;
}

// D: GTR
float microfacet(in float ndh, in float a2, in float power)
{
	const float ia2 = 1.0f - a2;
	const float denom = PI * pow((ndh * a2 - ndh) * ndh + 1.0f, power);
	return a2 / denom;
}

// Specular BRDF
vec3 brdf(in vec3 v, in vec3 n)
{	
	// Precalculate frequently used scalars.
	const vec3 h = normalize(v + Direction);
	const vec3 r = reflect(-v, n);
	const float ndl = mdot(n, Direction);
	const float ndv = mdot(n, v);
	const float ndh = mdot(n, h);
	const float vdh = dot(v, h);

	// Get all of the material values out of our G-Buffer.
	const vec4 diffRough = subpassLoad(GBufferDiffuseRough);
	const vec4 spec = subpassLoad(GBufferSpecular);
	const vec3 envi = texture(Environment, r).rgb;

	// Specular
	const vec3 f = fresnel(vdh, spec.xyz);
	const float g = occlusion(ndl, ndv, diffRough.w);
	const float d = microfacet(ndh, diffRough.w, spec.w);

	// Composition
	const vec3 fd = (1.0f - f) * (diffRough.rgb / PI);
	const vec3 fs = (f * g * d) * envi;
	return (fd + fs) * Radiance.rgb * Radiance.w * ndl;
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

void main()
{
	const vec3 p = DecodePosition();
	const vec3 n = DecodeNormal();
	const vec3 v = normalize(CamPos - p);

	L0 = vec4(brdf(v, n), 1.0f);
}