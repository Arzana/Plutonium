#version 460 core
#extension GL_KHR_vulkan_glsl : enable

// Defines the material that will be used for the normal models.
layout (set = 1, binding = 0) uniform sampler2D Diffuse;
layout (set = 1, binding = 1) uniform sampler2D SpecularGlossiness;
layout (set = 1, binding = 2) uniform sampler2D Bump;
layout (set = 1, binding = 3) uniform sampler2D Emissive;
layout (set = 1, binding = 4) uniform sampler2D Occlusion;
layout (set = 1, binding = 5) uniform Material
{
	vec4 F0Power;					// Stores the specular base (F0) and the specular power.
	vec4 DiffuseFactorRoughness;	// Stores the diffuse factor and roughness.
	float AlphaTheshold;			// Defines the point at which partially transparent pixels should be discarded.
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 Uv;
layout (location = 2) in mat3 TBN;

layout (location = 0) out vec4 GBufferDiffuseA2;	// Stores the Diffuse color and Roughness^2.
layout (location = 1) out vec4 GBufferSpecular;		// Stores the Specular color and power.
layout (location = 2) out vec2 GBufferNormal;		// Stores the normal in spherical world coorinates.
layout (location = 3) out vec4 GBufferEmissiveAO;	// Stores the (pre-multipled) emissve color and ambient occlusion.

// Encodes the normal in spherical coordinates.
// Optimized to use as little space and transformations as possible.
vec2 EncodeNormal()
{
	vec3 normal = normalize(TBN * normalize(texture(Bump, Uv).xyz * 2.0f - 1.0f));
	float phi = atan(normal.y, normal.x);
	return vec2(normal.z, phi);
}

void main()
{
	// Discard transparent pixels.
	const vec4 diffuse = texture(Diffuse, Uv) * vec4(DiffuseFactorRoughness.xyz, 1.0f);
	if (diffuse.a < AlphaTheshold) discard;

	// Set the first attachment.
	const vec4 specGloss = texture(SpecularGlossiness, Uv);
	const float roughness = (1.0f - specGloss.w) * DiffuseFactorRoughness.w;
	const float a2 = roughness * roughness;
	GBufferDiffuseA2 = vec4(diffuse.rgb, a2);

	// Set the second attachment.
	const vec3 specular = specGloss.rgb * F0Power.rgb;
	GBufferSpecular = vec4(specular, F0Power.w);

	// Set the third attachment.
	GBufferNormal = EncodeNormal();

	// Set the final attachment.
	const vec3 emissive = texture(Emissive, Uv).rgb;
	const float ao = texture(Occlusion, Uv).r;
	GBufferEmissiveAO = vec4(emissive, ao);
}