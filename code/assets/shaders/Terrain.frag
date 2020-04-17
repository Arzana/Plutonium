#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (early_fragment_tests) in;

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
	// Get the normalized height of the terrain.
	const float h = texture(Height, Uv2).r;
	const vec4 weights = texture(TextureMask, Uv2);

	// Calculate the diffuse based on the mask
	vec3 diffuse = vec3(0.0f);
	for (uint i = 0; i < 4; i++)
	{
		diffuse += weights[i] * texture(Textures, vec3(Uv1, i)).rgb;
	}

	// Roughness of terrain is always max, with no specular.
	GBufferDiffuseRough = vec4(diffuse, 1.0f);
	GBufferSpecular = vec4(vec3(0.0f), 2.0f);
	GBufferNormal = EncodeNormal();
}