#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (constant_id = 0) const float iGBufferWidth = 1.0f / 2560.0f;
layout (constant_id = 1) const float iGBufferHeight = 1.0f / 1440.0f;

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

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Attenuation;
layout (location = 2) in vec4 Radiance;

layout (location = 0) out vec4 L0;

float mdot(in vec3 a, in vec3 b)
{
	return max(0.0f, dot(a, b));
}

// Use quadratic polynomial for light falloff.
float attenuation(in float d)
{
	return 1.0f / (Attenuation.x + Attenuation.y * d + Attenuation.z * d * d);
}

// F: Schlick
vec3 fresnel(in float cosTheta, in vec3 f0)
{
	return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

// G: Smith Joint GGX (Vis)
float occlusion(in float ndh, in float ndl, in float ndv, in float vdh)
{
	const float gv = (2.0f * ndh * ndv) / vdh;
	const float gl = (2.0f * ndh * ndl) / vdh;
	return min(1.0f, min(gv, gl));
}

// D: GTR
float microfacet(in float ndh, in float a2, in float power)
{
	const float ia2 = 1.0f - a2;
	const float denom = PI * pow((ndh * a2 - ndh) * ndh + 1.0f, power);
	return a2 / denom;
}

// Specular BRDF
vec3 brdf(in vec3 v, in vec3 n, in vec3 p)
{	
	// Precalculate frequently used scalars.
	const vec3 l = normalize(Position - p);
	const vec3 h = normalize(v + l);
	const vec3 r = reflect(-v, n);
	const float ndl = mdot(n, l);
	const float ndv = mdot(n, v);
	const float ndh = mdot(n, h);
	const float vdh = mdot(v, h);

	// Calculate the light factors.
	const float a = attenuation(length(l));
	const vec3 intensity = a * Radiance.w * Radiance.rgb;

	// Get all of the material values out of our G-Buffer.
	const vec4 diffRough = subpassLoad(GBufferDiffuseRough);
	const vec4 spec = subpassLoad(GBufferSpecular);

	// Specular
	const vec3 f = fresnel(vdh, spec.xyz);
	const float g = occlusion(ndh, ndl, ndv, vdh);
	const float d = microfacet(ndh, diffRough.w, spec.w);

	// Composition
	const vec3 fd = (1.0f - f) * (diffRough.rgb / PI);
	const vec3 fs = (f * g * d) * (4.0f * ndl * ndv);
	return (fd + fs) * intensity * ndl;
}

// Decodes the normal from unsigned normalized to [0, 1]
vec3 DecodeNormal()
{
	return subpassLoad(GBufferNormal).xyz * 2.0f - 1.0f;
}

// Decodes the position from linear depth buffer.
vec3 DecodePosition()
{
	const vec2 uv = gl_FragCoord.xy * vec2(iGBufferWidth, iGBufferHeight);
	float ld = subpassLoad(GBufferDepth).r;
	vec4 ndc = vec4(uv * 2.0f - 1.0f, ld, 1.0f);
	vec4 eye = IProjection * ndc;
	eye /= eye.w;
	return (IView * eye).xyz;
}

void main()
{
	const vec3 p = DecodePosition();
	const vec3 n = DecodeNormal();
	const vec3 v = normalize(CamPos - p);

	L0 = vec4(brdf(v, n, p), 1.0f);
}