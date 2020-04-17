#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (early_fragment_tests) in;
#define SAMPLE_BASED_ON_HEIGHT

layout (set = 1, binding = 0) uniform sampler2D Height;
layout (set = 1, binding = 1) uniform sampler2D TextureMask;
layout (set = 1, binding = 2) uniform sampler2DArray Textures;

layout (set = 1, binding = 3) uniform Terrain
{
	mat4 Model;
	float Displacement;
	float Tessellation;
	float EdgeSize;
	float PatchSize;
};

layout (location = 0) in vec2 Uv1;
layout (location = 1) in vec2 Uv2;
layout (location = 2) in vec3 Normal;

layout (location = 0) out vec4 GBufferDiffuseRough;	// Stores the Diffuse color and Roughness.
layout (location = 1) out vec4 GBufferSpecular;		// Stores the Specular color and power.
layout (location = 2) out vec2 GBufferNormal;		// Stores the normal in spherical world coorinates.

// Encodes the normal in spherical coordinates.
// Optimized to use as little space and transformations as possible.
vec2 EncodeNormal()
{
	float phi = atan(Normal.y, Normal.x);
	return vec2(Normal.z, phi);
}

void main()
{
	// Sample either the height map or the mask to get the texture weights.
#ifdef SAMPLE_BASED_ON_HEIGHT
	const float h = texture(Height, Uv2).r;
#else
	const vec4 weights = texture(TextureMask, Uv2);
#endif

	// Calculate the diffuse based on the mask
	vec3 diffuse = vec3(0.0f);
	for (uint i = 0; i < 4; i++)
	{
#ifdef SAMPLE_BASED_ON_HEIGHT
		// Use the relative height of the terrain to determine the texture weight.
		const float rangeEnd = (i + 1) * 0.25f;
		const float weight = max(0.0f, (0.25f - abs(h - rangeEnd)) * 4.0f);
		diffuse += weight * texture(Textures, vec3(Uv1, i)).rgb;
#else
		// Sample using a predefined texture for weights.
		diffuse += weights[i] * texture(Textures, vec3(Uv1, i)).rgb;
#endif
	}

	// Roughness of terrain is always max, with no specular.
	GBufferDiffuseRough = vec4(diffuse, 1.0f);
	GBufferSpecular = vec4(vec3(0.0f), 2.0f);
	GBufferNormal = EncodeNormal();
}