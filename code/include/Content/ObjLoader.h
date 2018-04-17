#pragma once
#include <vector>
#include "Core\Math\Constants.h"
#include "Graphics\Color.h"

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
		/* The indices of the vertices. */
		std::vector<ObjLoaderVertex> Indices;
		/* The amount of vertices per face. */
		std::vector<size_t> VerticesPerFace;
		/* The materials associated with the faces. */
		std::vector<int64> Materials;
		/* The smoothing group associated with the faces. */
		std::vector<uint64> SmoothingGroups;

		/* Initializes a new instance of an obj loader mesh. */
		ObjLoaderMesh(void);
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
	};

	/* Defines the material information of a specified object. */
	struct ObjLoaderMaterial
	{
		/* The name of the material. */
		const char *Name;

		/* The ambient reflectivity of this material. */
		Color Ambient;
		/* The diffuse reflectivity of this material. */
		Color Diffuse;
		/* The specular reflectivity of this material. */
		Color Specular;
		/* The light transmission filter of this material. */
		Color Transmittance;
		/* Defines the specular highlight exponent (shininess) of this material. */
		float HighlightExponent;
		/* Defines the optical density (index of refraction) of this material. */
		float OpticalDensity;
		/* Defines the dissolve factor (transparency) of this material. */
		float Dissolve;

		/* Defines the ambient texture map associated with the material. */
		ObjLoaderTextureMap AmbientMap;
		/* Defines the diffuse texture map associated with the material. */
		ObjLoaderTextureMap DiffuseMap;
		/* Defines the specular texture map associated with the material. */
		ObjLoaderTextureMap SpecularMap;
		/* Defines the specular highlight map associated with the material. */
		ObjLoaderTextureMap HighlightMap;
		/* Defines the bump map associated with the material. */
		ObjLoaderTextureMap BumpMap;
		/* Defines the displacement map associated with the material. */
		ObjLoaderTextureMap DisplacementMap;
		/* Defines the alpha map associated with the material. */
		ObjLoaderTextureMap AlphaMap;
		/* Defines reflection map associated with the material. */
		ObjLoaderTextureMap ReflectionMap;

		/* Initializes a new instance of an obj loader material. */
		ObjLoaderMaterial(void);
	};

	/* Defines the intermediate result of the obj load process. */
	struct ObjLoaderResult
	{
		/* Contains all vertices defined within the obj model. */
		std::vector<Vector3> Vertices;
		/* Contains all the normals defined within the obj model. */
		std::vector<Vector3> Normals;
		/* Contains all the texture coordinates defined within the obj model. */
		std::vector<Vector3> TexCoords;
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