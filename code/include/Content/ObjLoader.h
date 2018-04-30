#pragma once
#include <vector>
#include "Core\Math\Constants.h"
#include "Graphics\Color.h"
#include "Core\Math\Vector2.h"

namespace Plutonium
{
	/* Defines the usable types of texture maps. */
	enum class ObjLoaderMapType
	{
		/* Default (non specified). */
		None,
		/* Basic sphere. */
		Sphere,
		/* Left side of a cube. */
		CubeLeft,
		/* Right side of a cube. */
		CubeRight,
		/* Front side of a cube. */
		CubeFront,
		/* Back side of a cube. */
		CubeBack,
		/* Top side of a cube. */
		CubeTop,
		/* Bottom side of a cube. */
		CubeBottom
	};

	/* Defines channels that can be used for obj textures. */
	enum class ObjLoaderChannel
	{
		/* No channel */
		None = '\0',
		/* Red channel. */
		Red = 'r',
		/* Green channel. */
		Greem = 'g',
		/* Blue channel. */
		Blue = 'b',
		/* Matte channel. */
		Matte = 'm',
		/* Luminance (default) channel. */
		Luminance = 'l',
		/* Z-Depth channel. */
		Depth = 'z'
	};

	/* Defines a vertex link within a mesh. */
	struct ObjLoaderVertex
	{
		/* The index of the vertex associated with this vertex. */
		int64 Vertex;
		/* The index of the normal associated with this vertex. */
		int64 Normal;
		/* The index of the texture coordinate associated with this vertex. */
		int64 TexCoord;

		/* Initializes a new instance of an obj loader vertex. */
		ObjLoaderVertex(void);
	};

	/* Defines the mesh information of a shape. */
	struct ObjLoaderMesh
	{
		/* The name of the shape. */
		const char *Name;
		/* The material associated with the mesh. */
		int64 Material;
		/* The indices of the vertices. */
		std::vector<ObjLoaderVertex> Indices;
		/* The smoothing group associated with the faces. */
		std::vector<uint64> SmoothingGroups;

		/* Initializes a new instance of an obj loader mesh. */
		ObjLoaderMesh(void);
		/* Copies a previous instance of an obj loader mesh. */
		ObjLoaderMesh(_In_ const ObjLoaderMesh &value);
		/* Moves a previous instance of an obj loader mesh. */
		ObjLoaderMesh(_In_ ObjLoaderMesh &&value);
		/* Releases the resources allocated by the mesh. */
		~ObjLoaderMesh(void);

		/* Copies a previous instance of an obj loader mesh. */
		_Check_return_ ObjLoaderMesh& operator =(_In_ const ObjLoaderMesh &other);
		/* Moves a previous instance of an obj loader mesh. */
		_Check_return_ ObjLoaderMesh& operator =(_In_ ObjLoaderMesh &&other);
	};

	struct ObjLoaderTextureMap
	{
		/* The relative path to the texture. */
		const char *Path;
		/* The type of the texture map. */
		ObjLoaderMapType Type;

		/* Defines the sharpness (clarity) of mip-mapped texture map. */
		float Sharpness;
		/* Defines the brightness (color gain/loss) of the texture map. */
		float Brightness;
		/* Defines the contrast (range gain/loss) of the texture map. */
		float Contrast;

		/* Defines the texture coordinate origin of the texture map. */
		Vector3 Origin;
		/* Defines the texture coordinate scale of the texture map. */
		Vector3 Scale;
		/* Defines the texture coordinate turbulence of the texture map. */
		Vector3 Turbulence;
		/* Specifies whether the texture coordinates are restricted to a 0-1 range. */
		bool ClampedCoords;

		/* Specifies whether blending in the horizontal direction is enabled. */
		bool BlendH;
		/* Specifies whether blending in the vertical direction is enabled. */
		bool BlendV;

		/* Specifies the channel used to create a scalar or bump texture map. */
		ObjLoaderChannel ScalarOrBumpChannel;
		/* Defines the bump map multiplier. */
		float BumpMod;

		/* Initializes a new instance of an obj loader texture map. */
		ObjLoaderTextureMap(_In_ bool isBump);
		/* Copies a previous instance of an obj loader texture map. */
		ObjLoaderTextureMap(_In_ const ObjLoaderTextureMap &value);
		/* Moves a previous instance of an obj loader texture map. */
		ObjLoaderTextureMap(_In_ ObjLoaderTextureMap &&value);
		/* Releases the resources allocated by the obj loader texture map. */
		~ObjLoaderTextureMap(void);

		/* Copies a previous instance of an obj loader texture map. */
		_Check_return_ ObjLoaderTextureMap& operator =(_In_ const ObjLoaderTextureMap &other);
		/* Moves a previous instance of an obj loader texture map. */
		_Check_return_ ObjLoaderTextureMap& operator =(_In_ ObjLoaderTextureMap &&other);
	};

	/* Defines the material information of a specified object. */
	struct ObjLoaderMaterial
	{
		/* The name of the material. */
		const char *Name;

		/*
		The ambient reflectivity of this material.
		Defines how the material reflects ambient light.
		Default: 'Black'.
		Token(s) (.mtl): 'Ka'.
		*/
		Color Ambient;
		/* 
		The diffuse reflectivity of this material.
		Defines how the material reflects diffuse light.
		Default: 'Black'.
		Token(s) (.mtl): 'Kd'.
		*/
		Color Diffuse;
		/* 
		The specular reflectivity of this material. 
		Defines how the material reflects specular light.
		Default: 'Black'.
		Token(s) (.mtl): 'Ks'.
		*/
		Color Specular;
		/* 
		The light transmission filter of this material. 
		Defines the color filter that should be applied to the final fragment color.
		Default: 'White'.
		Token(s) (.mtl): 'Kt', 'Tf'.
		*/
		Color Transmittance;
		/*
		The specular highlight exponent (shininess) of this material.
		Defines the focus of the specular highlight, a high value results in a concentrated highlight.
		Default: 1.0f.
		Token(s) (.mtl): 'Ns'.
		*/
		float HighlightExponent;
		/* 
		The optical density (index of refraction) of this material. 
		Defines how much light should bend whils passing through the object,
		a value of 1.0f means light doesn't bend, increasing this amount increases the bending,
		values below 1.0f produce unwanted results.
		Default: 1.0f.
		Token(s) (.mtl): 'Ni'.
		*/
		float OpticalDensity;
		/*
		The dissolve factor (transparency) of this material. 
		Defines how much an object using this material should blend in the background,
		a value of 1.0f means the object is fully opaque and 0.0f means it's fully transparent.
		Default: 1.0f.
		Token(s) (.mtl): 'd', 'Tr'.
		*/
		float Dissolve;
		/* 
		The illumination model.
		Defines which illumination model should be used whilst rendering with this material,
		0:	Color and ambient off.
		1:	Color on and ambient on.
		2:	Highlight on.
		3:	Reflection on and ray trace on.
		4:	Transparency glass on, reflection ray trace on.
		5:	Reflection fresnel on and ray trace on.
		6:	Transparency refraction on, reflection fresnel off and ray trace on.
		7:	Transparency refraction on, reflection fresnel on and ray trace on.
		8:	Reflection on and ray trace off.
		9:	Transparency glass on, reflection ray trace off.
		10:	Casts shadows onto invisible surfaces.
		Default: 0.
		Token(s) (.mtl): 'illum'.
		*/
		int IlluminationModel;

		/* 
		Defines the ambient texture map associated with the material. 
		Token(s) (.mtl): 'map_Ka'.
		*/
		ObjLoaderTextureMap AmbientMap;
		/* 
		Defines the diffuse texture map associated with the material. 
		Token(s) (.mtl): 'map_Kd'.
		*/
		ObjLoaderTextureMap DiffuseMap;
		/*
		Defines the specular texture map associated with the material. 
		Token(s) (.mtl): 'map_Ks'.
		*/
		ObjLoaderTextureMap SpecularMap;
		/* 
		Defines the specular highlight map associated with the material. 
		Token(s) (.mtl): 'map_Ns'.
		*/
		ObjLoaderTextureMap HighlightMap;
		/*
		Defines the bump map associated with the material. 
		Token(s) (.mtl): 'map_Bump', 'ma_bump', 'bump'.
		*/
		ObjLoaderTextureMap BumpMap;
		/* 
		Defines the displacement map associated with the material. 
		Token(s) (.mtl): 'disp'.
		*/
		ObjLoaderTextureMap DisplacementMap;
		/* 
		Defines the alpha map associated with the material. 
		Token(s) (.mtl): 'map_d'.
		*/
		ObjLoaderTextureMap AlphaMap;
		/*
		Defines reflection map associated with the material. 
		Token(s) (.mtl): 'refl'.
		*/
		ObjLoaderTextureMap ReflectionMap;

		/* Initializes a new instance of an obj loader material. */
		ObjLoaderMaterial(void);
		/* Copies a previous instance of an obj loader material. */
		ObjLoaderMaterial(_In_ const ObjLoaderMaterial &value);
		/* Moves a previous instance of an obj loader material. */
		ObjLoaderMaterial(_In_ ObjLoaderMaterial &&value);
		/* Releases the resources allocated by the obj loader material. */
		~ObjLoaderMaterial(void);

		/* Copies a previous instance of an obj loader material. */
		_Check_return_ ObjLoaderMaterial& operator =(_In_ const ObjLoaderMaterial &other);
		/* Moves a previous instance of an obj loader material. */
		_Check_return_ ObjLoaderMaterial& operator =(_In_ ObjLoaderMaterial &&other);
	};

	/* Defines the intermediate result of the obj load process. */
	struct ObjLoaderResult
	{
		/* Contains all vertices defined within the obj model. */
		std::vector<Vector3> Vertices;
		/* Contains all the normals defined within the obj model. */
		std::vector<Vector3> Normals;
		/* Contains all the texture coordinates defined within the obj model. */
		std::vector<Vector2> TexCoords;
		/* Contains all meshes defined within the obj model. */
		std::vector<ObjLoaderMesh> Shapes;
		/* Contains all materials defined within the obj model. */
		std::vector<ObjLoaderMaterial> Materials;

		/* Initializes a new instance of an obj loader result. */
		ObjLoaderResult(void);
	};

	/* Loads an obj file and it's associated mtl file(s) (requires delete!). */
	_Check_return_ const ObjLoaderResult* _CrtLoadObjMtl(_In_ const char *path);
}