#pragma once
#include "Core/Math/AABB.h"
#include "Graphics/Color.h"
#include "Streams/BinaryReader.h"
#include "Graphics/Resources/StagingBuffer.h"

namespace Pu
{
	class FileReader;

	/* Defines the types of joints present in a mesh. */
	enum class PumJointType
	{
		/* No joints or weights are present. */
		None = 0,
		/* Joint indices are stored using bytes. */
		Byte = 1,
		/* Joint indices are stored using unsigned shorts. */
		UShort = 2
	};

	/* Defines the types of indices present in a model (this type can be casted to Vulkan IndexType). */
	enum class PumIndexType
	{
		/* No index buffer is used. */
		None = 2,
		/* The index buffer stores UInt16's. */
		UInt16 = _CrtEnum2Int(IndexType::UInt16),
		/* The index buffer stores UInt32's. */
		UInt32 = _CrtEnum2Int(IndexType::UInt32)
	};

	/* Defines the types of interframe interpolation during an animation. */
	enum class PumInterpolationType
	{
		/* No interframe interpolation should be applied. */
		None = 0,
		/* Linear interframe interpolation should be applied. */
		Linear = 1,
		/*
		Hermite interframe interpolation should be applied.
		Arg1 = Tangent 1
		Arg2 = Tangent 2
		*/
		Cubic = 2,
		/*
		Linear interframe interpolation should be applied and
		a spring system should be used for the animation controller.
		Arg1 = Stiffness
		Arg2 = Damping
		*/
		Spring = 3
	};

	/* Defines the types of alpha blending. */
	enum class PumAlphaMode
	{
		/* The material should be rendered fully opaque. */
		Opaque = 0,
		/*
		The material should be rendered fully opaque if the fragment's alpha is below the threshold
		or the fragment should be discarded if it's above the threshold.
		*/
		Mask = 1,
		/* The material should be rendered using standard Porter & Duff blending. */
		Blend = 2
	};

	/* Defines a core point in the model file. */
	struct PumNode
	{
		/* Specifies the child nodes of this node. */
		vector<uint32> Children;

		/* Specifies whether the Mesh field is used. */
		bool HasMesh;
		/* Specifies whether the skin field is used. */
		bool HasSkin;
		/* Specifies whether the translation field is used. */
		bool HasTranslation;
		/* Specifies whether the rotation field is used. */
		bool HasRotation;
		/* Specifies whether the scale field is used. */
		bool HasScale;

		/* Defined the (optional) index to the mesh associated with this node. */
		uint32 Mesh;
		/* Defines the (optional) index to the skin associated with this node. */
		uint32 Skin;
		/* Defines the (optional) translation of this node. */
		Vector3 Translation;
		/* Defines the (optional) rotation of this node. */
		Quaternion Rotation;
		/* Defines the (optional) scale of this node. */
		Vector3 Scale;

		/* Default initializes a new instance of a PuM node. */
		PumNode(void);
		/* Initializes a new instance of a PuM node from a binary data stream. */
		PumNode(_In_ BinaryReader &reader);

		/* Gets the full transform of this node. */
		_Check_return_ Matrix GetTransform(void) const;
	};

	/* Defines a buffer view (bind) into the model GPU buffer. */
	struct PumView
	{
		/* Defines the offset (in bytes) from where the buffer view starts. */
		size_t Offset;
		/* Defines the size (in bytes) of the buffer view. */
		size_t Size;

		/* Default initializes a new instance of a PuM view. */
		PumView(void);
		/* Initializes a new instance of a PuM view with the specified values. */
		PumView(_In_ size_t offset, _In_ size_t size);
		/* Initializes a new instance of a PuM view from a binary data stream. */
		PumView(_In_ BinaryReader &reader);
	};

	/* Defines static geometry in the model. */
	struct PumMesh
	{
		/* Defines the name of the mesh. */
		ustring Identifier;
		/* Defines the bounding box of the mesh. */
		AABB Bounds;

		/* Specifies whether the Material field is used. */
		bool HasMaterial;
		/* Specifies whether the mesh's vertex format contains normals. */
		bool HasNormals;
		/* Specifies whether the mesh's vertex format contains tangents. */
		bool HasTangents;
		/* Specifies whether the mesh's vertex format contains texture uv's. */
		bool HasTextureCoordinates;
		/* Specifies whether the mesh's vertex format contains colors. */
		bool HasColors;
		/* Defines the type of joints in the mesh's vertex format. */
		PumJointType JointType;
		/* Defines the type of index buffer used. */
		PumIndexType IndexType;
		/* Defines how this mesh should be renderer (no all types are supported!). */
		PrimitiveTopology Topology;

		/* Defines the (optional) index to the material associated with this mesh. */
		uint32 Material;
		/* Defines the index of the vertex view that should be bound when rendering this model. */
		uint32 VertexView;
		/* Defines the index of the (optional) index view that should be bound when rendering this model. */
		uint32 IndexView;
		/* Defines where the vertex view starts (in bytes). */
		size_t VertexViewStart;
		/* Defines where the (optional) index view starts (in bytes). */
		size_t IndexViewStart;
		/* Defines the size (in bytes) of the vertex data. */
		size_t VertexViewSize;
		/* Defines the size (in bytes) of the index data. */
		size_t IndexViewSize;

		/* Default initializes a new instance of a PuM mesh. */
		PumMesh(void);
		/* Initializes a new instance of a PuM mesh from a binary stream. */
		PumMesh(_In_ BinaryReader &reader);

		/* Gets the stride (in bytes) of the vertices in this mesh. */
		_Check_return_ uint32 GetStride(void) const;
		/* Gets the stride (in bytes) of the indices in this mesh. */
		_Check_return_ uint32 GetIndexStride(void) const;
	};

	/* Defines a single skeletal animation frame for a specific node. */
	struct PumFrame
	{
		/* Defines the start of this frame (in seconds) relative to the animation start time. */
		float TimeStamp;
		/* Defines the translation applied to the node during this frame. */
		Vector3 Translation;
		/* Defines the rotation applied to the node during this frame. */
		Quaternion Rotation;
		/* Defines the scale applied to the node during this frame. */
		Vector3 Scale;
		/* Defines the bounds of the node's associated vertices during this frame. */
		AABB Bounds;

		/* Default initializes a new instance of a PuM frame. */
		PumFrame(void);
		/* Initializes a new instance of a PuM frame from a binary stream. */
		PumFrame(_In_ BinaryReader &reader);
	};

	/* Defines a sequence of skeletal animation frames for a specific node. */
	struct PumSequence
	{
		/* Defines the node affected by this sequence. */
		uint32 Node;
		/* Defines the frames of this sequence. */
		vector<PumFrame> Frames;

		/* Default intializes a new instance of a PuM sequence. */
		PumSequence(void);
		/* Initializes a new instance of a PuM sequence from a binary stream. */
		PumSequence(_In_ BinaryReader &reader);
	};

	/* Defines a single skeletatl or morph animation. */
	struct PumAnimation
	{
		/* Defines the name of the animation. */
		ustring Identifier;
		/* Defines the type of interpolation that should be used. */
		PumInterpolationType Interpolation;
		/* Defines whether the animation should loop. */
		bool ShouldLoop;
		/* Defines whether the animation should be played in reverse. */
		bool PlayInReverse;
		/* Defines whether the animation is defined in frames rather than sequences. */
		bool IsMorphAnimation;
		/* Defines whether sequence animations should be baked to morph animations. */
		bool ShouldBake;

		/* Defines the sequences of the animation (only set if IsMorphAnimation is false). */
		vector<PumSequence> Sequences;
		/* Defines the frames of the animation (only set if IsMorphAnimation is true). */
		vector<uint32> Frames;
		/* Defines the total play time of the animation. */
		float Duration;
		/* Defines the (optional) first argument for the interplation. */
		float Arg1;
		/* Defines the (optional) second argument for the interpolation. */
		float Arg2;

		/* Default initializes a new instance of a PuM animation. */
		PumAnimation(void);
		/* Initializes a new instance of a PuM animation from a binary stream. */
		PumAnimation(_In_ BinaryReader &reader);
	};

	/* Defines a single joint of a specific skeleton. */
	struct PumJoint
	{
		/* The node associated with this joint. */
		uint32 Node;
		/* The inverse bind matrix of this joint. */
		Matrix IBind;

		/* Default initializes a new instance of a PuM joint. */
		PumJoint(void);
		/* Initializes a new instance of a PuM joint from a binary stream. */
		PumJoint(_In_ BinaryReader &reader);
	};

	/* Defines a set of joints that are bound to nodes. */
	struct PumSkeleton
	{
		/* Defines the name of the skeleton. */
		ustring Identifier;
		/* Defines rhe root node of the skeleton. */
		uint32 Root;
		/* Defines the joints associated with the skeleton. */
		vector<PumJoint> Joints;

		/* Default initializes a new instance of a PuM skeleton. */
		PumSkeleton(void);
		/* Initializes a new instance of a PuM skeleon from a binary stream. */
		PumSkeleton(_In_ BinaryReader &reader);
	};

	/*
	Defines a PBR specular glossiness material.
	Emissive is calculated using 'texture(EmissiveTexture, Uv).r * EmmissiveFactor.rgb * EmmissiveIntensity'.
	*/
	struct PumMaterial
	{
		/* The name of the material. */
		ustring Identifier;

		/* Defnes whether the material should be rendered with backface culling disabled. */
		bool DoubleSided;
		/* Defines how the alpha channel should be treated. */
		PumAlphaMode AlphaMode;
		/* Defines whether the diffuse texture is used. */
		bool HasDiffuseTexture;
		/* Defines whether the specular glossiness texture is used. */
		bool HasSpecGlossTexture;
		/* Defines whether the normal texture is used. */
		bool HasNormalTexture;
		/* Defines whether the occlusion texture is used. */
		bool HasOcclusionTexture;
		/* Defines whether the emissive texture is used. */
		bool HasEmissiveTexture;

		/* Defines the diffuse color (or mask if a texture is defined) of the material. */
		Color DiffuseFactor;
		/* Defines the specular color (or mask if the texture is defined) of the material. */
		Color SpecularFactor;
		/* Defines the specular base color of the material. */
		Color EmissiveFactor;
		/* Defines the glossiness factor of the material. */
		float Glossiness;
		/* Defines the power used for the specular highlightes. */
		float SpecularPower;
		/* Defines the internsity of the emissive color. */
		float EmissiveIntensity;

		/* Defines the (optional) alpha threshold for mask alpha modes. */
		float AlphaTheshold;
		/* Defines the (optional) index to the diffuse texture. */
		uint32 DiffuseTexture;
		/* Defines the (optional) index to the specular glossiness texture. */
		uint32 SpecGlossTexture;
		/* Defines the (optional) index to the normal texture. */
		uint32 NormalTexture;
		/* Defines the (optional) index to the occlusion texture. */
		uint32 OcclusionTexture;
		/* Defines the (optional) index to the emissive texture. */
		uint32 EmissiveTexture;

		/* Default initializes a new instance of a PuM material. */
		PumMaterial(void);
		/* Initializes a new instance of a PuM material from a binary stream. */
		PumMaterial(_In_ BinaryReader &reader);
	};

	/* Defines a texture and its sampler options. */
	struct PumTexture
	{
		/* The relative path to the texture. */
		ustring Path;
		/* The filter used for magnification. */
		Filter Magnification;
		/* The filter used for minification. */
		Filter Minification;
		/* The filter used for choosing mipmaps. */
		SamplerMipmapMode MipMap;
		/* The address mode for the horizontal coordinates. */
		SamplerAddressMode AddressModeU;
		/* The address mode for the vertical coordinates. */
		SamplerAddressMode AddressModeV;
		/* Defines whether this texture should be loaded as an sRGB texture. */
		bool IsSRGB;

		/* Default initializes a new instance of a PuM texture. */
		PumTexture(void);
		/* Initializes a new instance of a PuM texture from a binary stream. */
		PumTexture(_In_ BinaryReader &reader);

		/* Constructs the create information for a sampler from the options. */
		_Check_return_ SamplerCreateInfo GetSamplerCreateInfo(void) const;
	};

	/* Defines the data loaded from a PuM file. */
	struct PuMData
	{
	public:
		/* Defines the version used by the file. */
		uint32 Version;
		/* Defines the display name for the model. */
		ustring Identifier;

		/* Defines the nodes of the model. */
		vector<PumNode> Nodes;
		/* Defines the buffer views of the model. */
		vector<PumView> Views;
		/* Defines the static geometry of the model. */
		vector<PumMesh> Geometry;
		/* Defines the animations of the model. */
		vector<PumAnimation> Animations;
		/* Defines skeletons of the model. */
		vector<PumSkeleton> Skeletons;
		/* Defines the materials of the model. */
		vector<PumMaterial> Materials;
		/* Defines the textures of the model. */
		vector<PumTexture> Textures;
		/* Defines the vertex and index data for the model (requires delete)! */
		StagingBuffer *Buffer;

		/* Initializes an empty instance of a Plutonium model. */
		PuMData(void);
		/* Initializes a new instance of a Plutonium model from a binary stream. */
		PuMData(_In_ LogicalDevice &device, _In_ BinaryReader &reader);

		/* Returns a PuM data object with only the version, identifier set and textures set. */
		_Check_return_ static PuMData TexturesOnly(_In_ const wstring &path);
		/* Returns a PuM data object with only the version, identifier, nodes, views, geometry and GPU buffer set. */
		_Check_return_ static PuMData MeshesOnly(_In_ LogicalDevice &device, _In_ const wstring &path);
		/* Returns a PuM data object with only the version, identifier and materials set. */
		_Check_return_ static PuMData MaterialsOnly(_In_ const wstring &path);

	private:
		static bool SetHeader(PuMData &result, FileReader &reader);
		static void Raise(const char *reason);
	};
}