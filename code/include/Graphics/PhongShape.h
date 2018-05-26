#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "Color.h"

namespace Plutonium
{
	struct AssetLoader;
	struct ObjLoaderMaterial;
	struct ObjLoaderTextureMap;

	/* Defines the internal layout of the model. */
	struct PhongShape
	{
	public:
		/* The name of the used material. */
		const char *MaterialName;
		/* The vertices of the model. */
		Mesh *Mesh;
		/* The ambient sampler of the material. */
		Texture *AmbientMap;
		/* The diffuse sampler of the material. */
		Texture *DiffuseMap;
		/* The specular sampler of the material. */
		Texture *SpecularMap;
		/* The transparency sampler of the material. */
		Texture *AlphaMap;
		/* The normal sampler of the material. */
		Texture *BumpMap;
		/* The color filter of the material. */
		Color Transmittance;
		/* The ambient reflectance of the material. */
		Color Ambient;
		/* The diffuse reflectance of the material. */
		Color Diffuse;
		/* The specular reflectance of the material. */
		Color Specular;
		/* The specular exponent of the material. */
		float SpecularExp;

		/* Initializes an empty instance of the phong shape object. */
		PhongShape(void);
		/* Initializes a new instance of a phong shape. */
		PhongShape(_In_ Plutonium::Mesh *mesh, _In_ const ObjLoaderMaterial *material, _In_ AssetLoader *loader);
		PhongShape(_In_ const PhongShape &value) = delete;
		PhongShape(_In_ PhongShape &&value) = delete;
		/* Releases the resources stored in the shape. */
		~PhongShape(void) noexcept;

		_Check_return_ PhongShape& operator =(_In_ const PhongShape &other) = delete;
		_Check_return_ PhongShape& operator =(_In_ PhongShape &&other) = delete;
	private:
		AssetLoader *loader;

		static void InitOptions(const ObjLoaderTextureMap *objOpt, TextureCreationOptions *texOpt);
		static Texture* CreateDefault(WindowHandler wnd, Color filler = Color::White);
	};
}