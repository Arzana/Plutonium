#pragma once
#include <string>
#include "Material.h"
#include "Content\ObjLoader.h"

namespace Plutonium
{
	/* Defines a Blinn-Phong material. */
	struct MaterialBP
		: public Material
	{
	public:
		/* The sampler to use for the ambient color. */
		Texture *Ambient;
		/* The sampler to use for the diffuse color. */
		Texture *Diffuse;
		/* The sampler to use for the specular color. */
		Texture *Specular;
		/* The sampler to use for the per fragment transparency. */
		Texture *Opacity;
		/* The sampler to use for per fragment normals. */
		Texture *Normal;
		/* The specular exponent to use. */
		float SpecularExponent;

		/* Initializes an empty instance of a Blinn-Phong material. */
		MaterialBP(_In_ const char *name, _In_ AssetLoader *loader);
		/* Initializes an instance of a Blinn_phong material with specified smplers. */
		MaterialBP(_In_ const char *name, _In_ AssetLoader *loader, _In_ Texture *ambient, _In_ Texture *diffuse, _In_ Texture *specular, _In_ Texture *opacity, _In_ Texture *normal);
		/* Initializes a new instance of a Blinn-Phong material from a .obj material blueprint. */
		MaterialBP(_In_ const ObjLoaderMaterial *blueprint, _In_ AssetLoader *loader);
		MaterialBP(_In_ const MaterialBP &value) = delete;
		MaterialBP(_In_ MaterialBP &&value) = delete;
		/* Releases the resources stored in the material. */
		~MaterialBP(void);

		_Check_return_ MaterialBP& operator =(_In_ const MaterialBP &other) = delete;
		_Check_return_ MaterialBP& operator =(_In_ MaterialBP &&other) = delete; 

	private:
		static void InitConfig(const ObjLoaderTextureMap *texture, Color filter, TextureCreationOptions *output);
		Texture* CreateDefault(const char *name, Color filler);
	};
}