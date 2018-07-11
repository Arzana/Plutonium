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
	struct PhongMaterial
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

#if defined (DEBUG)
		/* A unique random color assigned to the material to use in debug rendering (only available on debug mode!). */
		Color Debug;
#endif

		/* Initializes an empty instance of the phong shape object. */
		PhongMaterial(void);
		/* Initializes a new instance of a phong shape. */
		PhongMaterial(_In_ Plutonium::Mesh *mesh, _In_ const ObjLoaderMaterial *material, _In_ AssetLoader *loader);
		PhongMaterial(_In_ const PhongMaterial &value) = delete;
		PhongMaterial(_In_ PhongMaterial &&value) = delete;
		/* Releases the resources stored in the shape. */
		~PhongMaterial(void) noexcept;

		_Check_return_ PhongMaterial& operator =(_In_ const PhongMaterial &other) = delete;
		_Check_return_ PhongMaterial& operator =(_In_ PhongMaterial &&other) = delete;

		/* Creates a neutral material (requires delete!). */
		_Check_return_ static PhongMaterial* GetDefault(_In_ WindowHandler wnd);
	private:
		AssetLoader *loader;

		static void InitOptions(const ObjLoaderTextureMap *objOpt, TextureCreationOptions *texOpt);
		static Texture* CreateDefault(WindowHandler wnd, const char *name = "default", Color filler = Color::White);
	};
}