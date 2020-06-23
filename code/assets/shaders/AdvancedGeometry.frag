#version 460 core
#extension GL_KHR_vulkan_glsl : enable

// Defines the material that will be used for the normal models.
layout (set = 1, binding = 0) uniform sampler2D Diffuse;
layout (set = 1, binding = 1) uniform sampler2D SpecularGlossiness;
layout (set = 1, binding = 2) uniform sampler2D Bump;
layout (set = 1, binding = 3) uniform Material
{
	vec4 F0Power;					// Stores the specular base (F0) and the specular power.
	vec4 DiffuseFactorRoughness;	// Stores the diffuse factor and roughness.
	float AlphaTheshold;			// Defines the point at which partially transparent pixels should be discarded.
};

layout (location = 0) in vec2 Uv;
layout (location = 1) in mat3 TBN;

layout (location = 0) out vec4 GBufferDiffuseRough;	// Stores the Diffuse color and Roughness.
layout (location = 1) out vec4 GBufferSpecular;		// Stores the Specular color and power.
layout (location = 2) out vec2 GBufferNormal;		// Stores the normal in spherical world coorinates.

// Encodes the normal in Lambert Azimuthal Equal-Area projection.
vec2 EncodeNormal()
{
	const vec3 n = normalize(TBN * normalize(textureLod(Bump, Uv, 0.0f).xyz * 2.0f - 1.0f));
	const float f = sqrt(8.0f * n.z + 8.0f);
	return n.xy / f + 0.5f;
}

void main()
{
	// Discard transparent pixels.
	const vec4 diffuse = texture(Diffuse, Uv) * vec4(DiffuseFactorRoughness.xyz, 1.0f);
	if (diffuse.a < AlphaTheshold) discard;

	// Set the first attachment.
	const vec4 specGloss = texture(SpecularGlossiness, Uv);
	const float roughness = (1.0f - specGloss.w) * DiffuseFactorRoughness.w;
	GBufferDiffuseRough = vec4(diffuse.rgb, roughness);

	// Set the second attachment.
	const vec3 specular = specGloss.rgb * F0Power.rgb;
	GBufferSpecular = vec4(specular, F0Power.w);

	// Set the third attachment.
	GBufferNormal = EncodeNormal();
}