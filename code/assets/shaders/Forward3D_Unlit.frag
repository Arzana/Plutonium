#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 1) uniform Camera
{
	vec3 CamPos;
	float Brightness;
	float Contrast;
	float Exposure;
};

layout (binding = 2) uniform samplerCube Environment;
layout (binding = 0, set = 1) uniform sampler2D Diffuse;
layout (binding = 1, set = 1) uniform sampler2D SpecularGlossiness;
layout (binding = 2, set = 1) uniform sampler2D Normal;
layout (binding = 3, set = 1) uniform sampler2D Emissive;
layout (binding = 4, set = 1) uniform sampler2D Occlusion;

// We could store the glossiness in the diffuse factor to save 4 bytes due to allignment.
layout (binding = 5, set = 1) uniform Material
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

void main()
{
	// Discard transparent pixels.
	const vec4 diff = texture(Diffuse, Uv) * vec4(DiffuseFactor, 1.0f);
	if (diff.a < 0.5f) discard;

	// Get emissive light and ambient occlusion.
	const float ao = texture(Occlusion, Uv).r;
	const vec3 emissive = texture(Emissive, Uv).rgb;
	
	// Compose the final output color.
	const vec3 hdr = (diff.rgb * ao + emissive) * Contrast + Brightness;
	L0 = vec4(1.0f - exp(-hdr * Exposure), 1.0f);
}