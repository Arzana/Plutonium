#pragma once
#include "Core/String.h"
#include "Graphics/Color.h"
#include "PumData.h"

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
	Pu::int64 Vertex;
	/* The index of the normal associated with this vertex. */
	Pu::int64 Normal;
	/* The index of the texture coordinate associated with this vertex. */
	Pu::int64 TexCoord;

	/* Initializes a new instance of an obj loader vertex. */
	ObjLoaderVertex(void);
};

/* Defines the mesh information of a shape. */
struct ObjLoaderMesh
{
	/* The name of the shape. */
	Pu::string Name;
	/* The material associated with the mesh. */
	Pu::int64 Material;
	/* The indices of the vertices. */
	Pu::vector<ObjLoaderVertex> Indices;
	/* The smoothing group associated with the faces. */
	Pu::vector<Pu::uint64> SmoothingGroups;

	/* Initializes a new instance of an obj loader mesh. */
	ObjLoaderMesh(void);
};

struct ObjLoaderTextureMap
{
	/* The relative path to the texture. */
	Pu::string Path;
	/* The type of the texture map. */
	ObjLoaderMapType Type;

	/* Defines the sharpness (clarity) of mip-mapped texture map. */
	float Sharpness;
	/* Defines the brightness (color gain/loss) of the texture map. */
	float Brightness;
	/* Defines the contrast (range gain/loss) of the texture map. */
	float Contrast;

	/* Defines the texture coordinate origin of the texture map. */
	Pu::Vector3 Origin;
	/* Defines the texture coordinate scale of the texture map. */
	Pu::Vector3 Scale;
	/* Defines the texture coordinate turbulence of the texture map. */
	Pu::Vector3 Turbulence;
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

	/* Checks if the two specified texture maps have the same parameters. */
	_Check_return_ bool operator !=(_In_ const ObjLoaderTextureMap &other) const;
};

/* Defines the material information of a specified object. */
struct ObjLoaderMaterial
{
	/* The name of the material. */
	Pu::string Name;

	/*
	The ambient reflectivity of this material.
	Defines how the material reflects ambient light.
	Default: 'Black'.
	Token(s) (.mtl): 'Ka'.
	*/
	Pu::Color Ambient;
	/*
	The diffuse reflectivity of this material.
	Defines how the material reflects diffuse light.
	Default: 'Black'.
	Token(s) (.mtl): 'Kd'.
	*/
	Pu::Color Diffuse;
	/*
	The specular reflectivity of this material.
	Defines how the material reflects specular light.
	Default: 'Black'.
	Token(s) (.mtl): 'Ks'.
	*/
	Pu::Color Specular;
	/*
	The light transmission filter of this material.
	Defines the color filter that should be applied to the final fragment color.
	Default: 'White'.
	Token(s) (.mtl): 'Kt', 'Tf'.
	*/
	Pu::Color Transmittance;
	/*
	The light emitting from this materials surface.
	Defines the color that should be emitted by the surface of this material.
	Default: 'Black'.
	Token(s) (.mtl): 'Ke'
	*/
	Pu::Color Emissive;
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
	The global glossiness (or roughness) modifier of this material.
	Defines the smoothness of the microsurface level, reflected light will scatter more
	if the surface is rougher, a value of 1.0f means the object will hardly scatter and
	a 0.0f mean the object will scatter all it's reflected light.
	Note that this value needs to be inverted if used in a shader to convert to roughtness.
	Default: 1.0f.
	Token(s) (.mtl): 'Pr'.
	*/
	float Glossiness;
	/*
	The global metallic factor used for this material.
	Defines the depth that the refracted light will penetrate the material, a value of 1.0f
	means the material is fully made of metal and thusly light will not travel deep within it,
	a value of 0.0f would indicate a dielectric material where light will travel far within it.
	Default: 0.0f.
	Token(s) (.mtl): 'Pm'.
	*/
	float Metallic;
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
	Defines the  albedo (or diffuse) texture map associated with the material.
	Token(s) (.mtl): 'map_Kd'.
	*/
	ObjLoaderTextureMap AlbedoMap;
	/*
	Defines the specular texture map associated with the material.
	Token(s) (.mtl): 'map_Ks'.
	*/
	ObjLoaderTextureMap SpecularMap;
	/*
	Defines the emissive texture map associated with the material.
	Token(s) (.mtl): 'map_Ke'.
	*/
	ObjLoaderTextureMap EmissiveMap;
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
	/*
	Defines the glossiness map associated with the material.
	Token(s) (.mtl): 'map_Pr'.
	*/
	ObjLoaderTextureMap GlossinessMap;
	/*
	Defines the metaliic map associated with the material.
	Token(s) (.mtl): 'map_Pm'
	*/
	ObjLoaderTextureMap MetallicMap;

	/* Initializes a new instance of an obj loader material. */
	ObjLoaderMaterial(void);
};

/* Defines the intermediate result of the obj load process. */
struct ObjLoaderResult
{
	/* Contains all vertices defined within the obj model. */
	Pu::vector<Pu::Vector3> Vertices;
	/* Contains all the normals defined within the obj model. */
	Pu::vector<Pu::Vector3> Normals;
	/* Contains all the texture coordinates defined within the obj model. */
	Pu::vector<Pu::Vector2> TexCoords;
	/* Contains all meshes defined within the obj model. */
	Pu::vector<ObjLoaderMesh> Shapes;
	/* Contains all materials defined within the obj model. */
	Pu::vector<ObjLoaderMaterial> Materials;

	/* Initializes a new instance of an obj loader result. */
	ObjLoaderResult(void);

	/* Gets the index of the default material of this obj; -1 if none is defined. */
	_Check_return_ Pu::int32 GetDefaultMaterial(void) const;
};

/* Loads an obj file and it's associated mtl file(s) (requires delete!). */
void LoadObjMtl(_In_ const Pu::string &path, _Out_ ObjLoaderResult &result);
void ObjToPum(_In_ ObjLoaderResult &input, _Out_ PumIntermediate &result);