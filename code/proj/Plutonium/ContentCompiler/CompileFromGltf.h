#pragma once
#include "CommandLineArguments.h"
#include "PumData.h"
#include <Core/Math/Interpolation.h>

/* Defines the component type of a buffer value. */
enum class GlTfComponentType
{
	/* Signed byte. */
	Int8 = 5120,
	/* Unsigned byte. */
	UInt8 = 5121,
	/* Short. */
	Int16 = 5122,
	/* Unsigned short. */
	UInt16 = 5123,
	/* Int. */
	Int32 = 5124,
	/* Unsigned int. */
	UInt32 = 5125,
	/* Float. */
	Float = 5126,
	/* Double. */
	Double = 5130
};

/* Defines the type of a buffer value. */
enum class GlTfType
{
	/* 2D vector. */
	Vector2 = 2,
	/* 3D vector. */
	Vector3 = 3,
	/* 4D vector. */
	Vector4 = 4,
	/* 2x2 matrix. */
	Matrix2 = 34,
	/* 3x3 matrix. */
	Matrix3 = 35,
	/* 4x4 matrix. */
	Matrix4 = 36,
	/* Single scalar value. */
	Scalar = 65,
	/* List of values. */
	Vector = 66,
	/* Generic matrix. */
	Matrix = 80
};

/* Defines the types of render modes that a GLTF primitive can have. */
enum class GLTFMode
{
	/* A list of points. */
	Points = 0,
	/* A list of lines. */
	Line = 1,
	/* A looped list of lines. */
	LineLoop = 2,
	/* A list of triangles. */
	Triangles = 4,
	/* A strip of triangles. */
	TriangleStrip = 5,
	/* A fan of triangles. */
	TriangleFan = 6
};

/* Defines the targets for animation frames. */
enum class GLTFAnimationTarget
{
	/* The translation component (accessor must be 3D float vector). */
	Translation,
	/*
	The rotation component (accessor must be 4D).
	Types can either be:
		float			(pass directly)
		byte			(pass as max(c / 127.0f, -1.0f))
		unsigned byte	(pass as c / 255.0f)
		short			(pass as max(c / 32767.0f, -1.0f))
		unsigned short	(pass as c / 65535.0f)
	*/
	Rotation,
	/* The scale component (accessor must be 3D float vector). */
	Scale,
	/* The joint weights component (accessor must be scalar, for types, see Rotation). */
	Weights
};

/* Defines the types of inter-frame interpolation in a GLTF sampler. */
enum class GLTFInterpolation
{
	/* Step by step (no interpolation). */
	Step = 0,
	/* Linear interpolation. */
	Linear = 1,
	/* Cubic hermite spline interpolation. */
	Cubic = 2
};

/* Defines the types of texture filtering in a GLTF sampler. */
enum class GLTFFilter
{
	/* Nearest fragment sampling. */
	Nearest = 9728,
	/* Linear color sampling. */
	Linear = 9729,
	/* Nearest fragment sampling on nearest mipmap. */
	NearestMipmapNearest = 9984,
	/* Nearest fragment sampling on linear interpolated mipmap. */
	NearestMipmapLinear = 9986,
	/* Linear color sampling on nearest mipmap. */
	LinearMipmapNearest = 9985,
	/* Linear color sampling on linear interpolated mipmap. */
	LinearMipmapLinear = 9987
};

/* Defines the types of texture wrapping ina GLTF sampler. */
enum class GLTFWrap
{
	/* Clamp the last valid fragment to the edge. */
	ClampToEdge = 33071,
	/* Output a solid border color. */
	ClampToBorder = 33069,
	/* Repeast the coordinates. */
	Repeat = 10497,
	/* Mirrors the coordinates. */
	MirroredRepeat = 33648,
};

/* Defines the types of attributes that can be stored in a GLTF primitive. */
enum class GLTFPrimitiveAttribute
{
	/* A vertex position (must be a Vector3). */
	Position,
	/* The normalized vertex normal (must be a Vector3). */
	Normal,
	/*
	The normalized vertex tangent (must be a Vector4).
	W components is a sign indicating the handedness of the tangent basis.
	Bitangent can be computed via cross(normal, tangent.XYZ) * tangent.W
	*/
	Tangent,
	/*
	The texture UV for the first set (must be a 2D).
	Can either be stored as floats for direct usage,
	unsigned byte/short needs to be converted using value/max.
	*/
	TexCoord1,
	/* The texture UV for any additional texture coordinate (see TexCoord1). */
	TexCoordAux,
	/*
	The RGB or RGBA vertex color.
	Can either be stored as floats or unsigned normalized values (see TexCoord1).
	*/
	Color,
	/* The joint indices of the vertex (must be 4D and unsigned byte/short). */
	Joints,
	/*
	The weights of the joints (Must be 4D).
	Can either be stored as floats or unsigned normalized values (see TexCoord1).
	*/
	Weights,
	/* Any primitive attribute not recognized by Plutonium. */
	Other
};

/* Defines an accessor in the GLTF format. */
struct GLTFAccessor
{
public:
	/* The bufferview associated with the accessor. */
	size_t BufferView;
	/* The offset (in bytes) added by the accessor to the buffer data. */
	size_t Start;
	/* The amount of elements that belong to this accessor. */
	size_t Count;
	/* The underlying type of the value(s) that make up the type. */
	GlTfComponentType ComponentType;
	/* The type of the buffer value(s). */
	GlTfType Type;
	/* The minimum value of the type. */
	Pu::vector<double> Minimum;
	/* The maximum value of the type. */
	Pu::vector<double> Maximum;

	/* Initializes a default instance of a GLTF accessor. */
	GLTFAccessor(void)
		: BufferView(0), Start(0), Count(0)
	{}

	/* Gets the size (in bytes) of a single element. */
	_Check_return_ size_t GetElementSize(void) const;
	/* Gets the size (in bytes) of the entire accessor. */
	_Check_return_ size_t GetSize(void) const;
};

/* Defines the information about the buffers data. */
struct GLTFBufferView
{
	/* The buffer index. */
	size_t Buffer;
	/* The offset (in bytes) added by the buffer view to the buffer data. */
	size_t Start;
	/* The length (in bytes) of the buffer available to this view. */
	size_t Length;

	/* Initializes a default instance of a GLTF buffer view. */
	GLTFBufferView(void)
		: Buffer(0), Start(0), Length(0)
	{}
};

/* Defines the target information for an animation sampler. */
struct GLTFChannel
{
	/* The sampler associated with the channel. */
	size_t Sampler;
	/* The node that is affected by the sampler. */
	size_t TargetNode;
	/* The channel targeted by the animation sampler. */
	GLTFAnimationTarget Target;
};

/* Defines the frame sampler for an animation. */
struct GLTFAnimationSampler
{
	/* The input sampler index (defines time stamps). */
	size_t Input;
	/* The output sampler index (defines channel frames). */
	size_t Output;
	/* The type of inter-frame interpolation to apply. */
	GLTFInterpolation Interpolation;
};

/* Defines a named animation in the GlTF format. */
struct GLTFAnimation
{
	/* The name of the animation. */
	Pu::string Name;
	/* The channels of the animation. */
	Pu::vector<GLTFChannel> Channels;
	/* The samplers of the animation. */
	Pu::vector<GLTFAnimationSampler> Samplers;
};

/* Defines a raw data buffer. */
struct GLTFBuffer
{
	/* The source of this buffer. */
	Pu::wstring Uri;
	/* The size (in bytes) of the buffers data. */
	size_t Size;
};

/*
Defines a generic material parameter in the GLTF format, known values:
pbrMetallicRoughness
	baseColorFactor:			Defines a mask for the albedo.
	baseColorTexture:			Defines a texture index (and optionally the texture coordinates) for the albedo.
	metallicFactor:				Defines a mask for the metallic.
	roughnessFactor:			Defines a scalar for the roughness.
	metallicRoughnessTexture:	Defines a combined texture index for the metallic and roughtness.
KHR_materials_pbrSpecularGlossiness
	diffuseFactor:				Defines a mask for the diffuse albedo.
	diffuseTexture:				Defines a texture index for the diffuse albedo.
	specularFactor:				Defines a mask for the specular albedo.
	glossinessFactor:			Defines a scalar for the glossiness.
	specularGlossinessTexture:	Defines a combined texture index for the specular albedo and glossiness.
emissiveFactor:				Defines an extra emissive mask.
emissiveTexture:			Defines an extra texture index for the emissiveness.
normalTexture:				Defines an extra texture index for bump mapping.
occlusionTexture:			Defines an extra texture index for basic ambient occlusion.
alphaMode:					Defines how the material should be blended (OPAQUE = no blending, MASK = either full opaque or transparent based on alpha, BLEND = Apply Porter and Duff)
alphaCutoff:				Defines the threshold for alpha cutoff if mask is specified as the alpha mode.
doubleSided:				If this optional value is set to true back-culling should be disabled, otherwise; it should be enabled.
*/
struct GLTFParameter
{
	/* The value of an unnamed boolean value. */
	bool BooleanValue;
	/* Defines whether a numeric value has been specified. */
	bool HasNumberValue;
	/* The value of an unnamed string value. */
	Pu::string StringValue;
	/* A list of unnamed numerical values defined within the parameter. */
	Pu::vector<double> Numbers;
	/* A list of named numerical values defined within the paramter. */
	std::map<Pu::string, double> NamedNumbers;
	/* A single unnamed numeric value. */
	double NumberValue;

	/* Initializes a new instance of a GLTF material parameter. */
	GLTFParameter(void)
		: BooleanValue(false), HasNumberValue(false), NumberValue(0.0)
	{}

	/* Attempt to retrieve a texture index and the texture coordinate map from this parameter. */
	_Check_return_ inline bool TryGetTextureInfo(_Out_ size_t &index, _Out_ size_t &map) const
	{
		std::map<Pu::string, double>::const_iterator it = NamedNumbers.find("INDEX");
		if (it == NamedNumbers.end()) return false;
		index = static_cast<size_t>(it->second);

		it = NamedNumbers.find("TEXCOORD");
		map = it != NamedNumbers.end() ? static_cast<size_t>(it->second) : 0;

		return true;
	}

	/* Attempt to retrieve a named number from this parameter. */
	_Check_return_ inline bool TryGetNamedNumber(_In_ const Pu::string &name, _Out_ double &value) const
	{
		std::map<Pu::string, double>::const_iterator it = NamedNumbers.find(name);
		if (it == NamedNumbers.end()) return false;

		value = it->second;
		return true;
	}

	/* Converts the values in the number vector to an RGBA color. */
	_Check_return_ inline Pu::Color GetColor(void) const
	{
		const float r = static_cast<float>(Numbers[0]);
		const float g = static_cast<float>(Numbers[1]);
		const float b = static_cast<float>(Numbers[2]);
		const float a = Numbers.size() > 3 ? static_cast<float>(Numbers[3]) : 1.0f;
		return Pu::Color(r, g, b, a);
	}

	/* Converts the values in the number vector to an RGBA color, normalizes it and returns the intensity. */
	_Check_return_ inline Pu::Color GetHDRColor(_Out_ float &intensity) const
	{
		const float r = static_cast<float>(Numbers[0]);
		const float g = static_cast<float>(Numbers[1]);
		const float b = static_cast<float>(Numbers[2]);
		const float a = Numbers.size() > 3 ? static_cast<float>(Numbers[3]) : 1.0f;

		intensity = Pu::ilerp(0.0f, 1.0f, Pu::max(r, Pu::max(g, b)));
		return Pu::Color(r / intensity, g / intensity, b / intensity, a);
	}
};

/* Defines a named list of material paramters. */
struct GLTFMaterial
{
	/* The name of the material. */
	Pu::string Name;
	/* The values defined in the material (all keys are in upper case). */
	std::map<Pu::string, GLTFParameter> Values;

	/* Checks whether a specified value is present in the value list. */
	_Check_return_ bool ContainsValue(_In_ const Pu::string &key) const
	{
		return Values.find(key.toUpper()) != Values.end();
	}

	/* Attempts to get a specific parameter from the material, returns true when successful, otherwise; false. */
	_Check_return_ bool TryGetValue(_In_ const Pu::string &key, _Out_ GLTFParameter &result) const
	{
		std::map<Pu::string, GLTFParameter>::const_iterator it = Values.find(key.toUpper());
		if (it != Values.end())
		{
			result = it->second;
			return true;
		}

		return false;
	}
};

/* Defines the primitive attributes of a mesh. */
struct GLTFPrimitive
{
	/* Defines the named attribute of this primitive and it's accessor index. */
	std::multimap<GLTFPrimitiveAttribute, size_t> Attributes;
	/* Defines an optional material index to apply to this primitive. */
	size_t Material;
	/* The index of the accessor that defines the indeces of this primitve (optional). */
	size_t Indices;
	/* Defines how to render this primitive. */
	GLTFMode Mode;
	/* Specifies whether the material index is valid. */
	bool HasMaterial;
	/* Specifies whether the indices index is valid. */
	bool HasIndices;

	/* Intializes a new instance of a GLTF primitive. */
	GLTFPrimitive(void)
		: Material(0), Indices(0), HasMaterial(false), HasIndices(false), Mode(GLTFMode::Triangles)
	{}

	/* Attempts to get the specified attribute, returns true if found, otherwise; false. */
	_Check_return_ inline bool TryGetAttribute(_In_ GLTFPrimitiveAttribute type, _Out_ size_t &accessorIdx) const
	{
		std::map<GLTFPrimitiveAttribute, size_t>::const_iterator it = Attributes.find(type);
		if (it != Attributes.end())
		{
			accessorIdx = it->second;
			return true;
		}

		return false;
	}
};

/* Defines mesh parameters in the GLTF format. */
struct GLTFMesh
{
	/* The name of the mesh. */
	Pu::string Name;
	/* The primitives defined in this mesh. */
	Pu::vector<GLTFPrimitive> Primitives;
};

/* Defines a node in the scene. */
struct GLTFNode
{
	/* The name of this node (optional). */
	Pu::string Name;
	/* The skin index associated with this node (optional). */
	size_t Skin;
	/* The mesh index associated with the node (optional). */
	size_t Mesh;
	/* The child nodes of this node. */
	Pu::vector<size_t> Children;
	/* The translation vector defined in this node (optional). */
	Pu::Vector3 Translation;
	/* The rotation quaternion defined in this node (optional). */
	Pu::Quaternion Rotation;
	/* The scalar vector defined in this node (optional). */
	Pu::Vector3 Scale;
	/* Specifies whether the skin index is valid. */
	bool HasSkin;
	/* Specifies whether the mesh index is valid. */
	bool HasMesh;

	/* Initializes a new instance of a GLTF node. */
	GLTFNode(void)
		: Skin(0), Mesh(0), HasSkin(false), HasMesh(false), Scale(1.0f)
	{}
};

/* Defines a named texture sampler in the GLTF format. */
struct GLTFTexture
{
	/* The index of the sampler associated with the texture. */
	size_t Sampler;
	/* The index of the source image associated with the texture. */
	size_t Image;

	/* Initializes a default instance of a GLTF texture. */
	GLTFTexture(void)
		: Sampler(0), Image(0)
	{}
};

/* Defines a vertex skin and skeleton in the GLTF format. */
struct GLTFSkin
{
	/* The name of the skin (optional). */
	Pu::string Name;
	/* The index of the accessor that stores the inverse bind matrices. */
	size_t IBindMatrices;
	/* The index of the node used as the skeleton root. */
	size_t Skeleton;
	/* The indices of the skeleton nodes. */
	Pu::vector<size_t> Joints;
};

/* Defines the options for texture samplers. */
struct GLTFImageSampler
{
	/* The minifying filter. */
	GLTFFilter MinFilter;
	/* The magnifying filter. */
	GLTFFilter MagFilter;
	/* The horizontal wrap. */
	GLTFWrap WrapS;
	/* The vertical wrap. */
	GLTFWrap WrapT;

	/* Initializes the default GLTF images sampler. */
	GLTFImageSampler(void)
		: MinFilter(GLTFFilter::LinearMipmapLinear), MagFilter(GLTFFilter::LinearMipmapLinear),
		WrapS(GLTFWrap::Repeat), WrapT(GLTFWrap::Repeat)
	{}
};

/* Defines a scene in the GLTF format. */
struct GLTFScene
{
	/* The name of the scene (optional). */
	Pu::string Name;
	/* The nodes that contain the information about the scene. */
	Pu::vector<size_t> Nodes;
};

/* Defines the result of a GLTF load. */
struct GLTFLoaderResult
{
	/* Defines the default scene to display. */
	size_t DefaultScene;
	/* Defines the available buffer accessors. */
	Pu::vector<GLTFAccessor> Accessors;
	/* Defines the available animations. */
	Pu::vector<GLTFAnimation> Animations;
	/* Defines the available binary buffers. */
	Pu::vector<GLTFBuffer> Buffers;
	/* Defines the available buffer views. */
	Pu::vector<GLTFBufferView> BufferViews;
	/* Defines the available materials. */
	Pu::vector<GLTFMaterial> Materials;
	/* Defines the available meshes. */
	Pu::vector<GLTFMesh> Meshes;
	/* Defines the available scene nodes. */
	Pu::vector<GLTFNode> Nodes;
	/* Defines the available textures. */
	Pu::vector<GLTFTexture> Textures;
	/* Defines the available image sources. */
	Pu::vector<Pu::wstring> Images;
	/* Defines the avialble vertex skins. */
	Pu::vector<GLTFSkin> Skins;
	/* Defines the available samplers. */
	Pu::vector<GLTFImageSampler> Samplers;
	/* Defines the availalbe scenes. */
	Pu::vector<GLTFScene> Scenes;

	/* Initializes a new instance of a GLTF file. */
	GLTFLoaderResult(void)
		: DefaultScene(0)
	{}
};

/* Loads the raw GLTF data from file. */
void LoadGLTF(_In_ const CLArgs &args, _Out_ GLTFLoaderResult &file);
void GltfToPum(const CLArgs &args, const GLTFLoaderResult &input, PumIntermediate &result);