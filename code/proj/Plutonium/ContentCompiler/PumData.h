#pragma once
#include <Streams/BinaryWriter.h>
#include <Core/Math/AABB.h>
#include <Graphics/Color.h>

struct pum_node
{
	Pu::vector<Pu::uint32> Children;

	bool WriteMeshIndex;
	bool WriteSkinIndex;
	bool WriteTranslation;
	bool WriteRotation;
	bool WriteScale;

	Pu::uint32 Mesh;
	Pu::uint32 Skin;

	Pu::Vector3 Translation;
	Pu::Quaternion Rotation;
	Pu::Vector3 Scale;

	pum_node(void)
		: WriteMeshIndex(false), WriteSkinIndex(false), WriteTranslation(false), 
		WriteRotation(false), WriteScale(false)
	{}

	inline void SetMesh(Pu::uint32 mesh)
	{
		WriteMeshIndex = true;
		Mesh = mesh;
	}

	inline void SetSkin(Pu::uint32 skin)
	{
		WriteSkinIndex = true;
		Skin = skin;
	}

	inline void SetTranslation(Pu::Vector3 translation)
	{
		WriteTranslation = true;
		Translation = translation;
	}

	inline void SetRotation(Pu::Quaternion rotation)
	{
		WriteRotation = true;
		Rotation = rotation;
	}

	inline void SetScale(Pu::Vector3 scale)
	{
		WriteScale = true;
		Scale = scale;
	}

	inline Pu::byte GetFlags(void) const
	{
		Pu::byte result = 0;

		if (WriteMeshIndex) result |= 0x01;
		if (WriteSkinIndex) result |= 0x02;
		if (WriteTranslation) result |= 0x04;
		if (WriteRotation) result |= 0x08;
		if (WriteScale) result |= 0x10;

		return result;
	}
};

struct pum_mesh
{
	Pu::ustring Identifier;
	Pu::AABB Bounds;

	bool WriteMaterialIndex;
	bool HasNormals;
	bool HasTangents;
	bool HasTextureUvs;
	bool HasVertexColors;
	char HasJoints;
	char IndexMode;
	char Topology;

	size_t VertexViewStart;
	size_t VertexViewSize;
	Pu::uint32 Material;
	size_t IndexViewStart;
	size_t IndexViewSize;

	pum_mesh(void)
		: WriteMaterialIndex(false), HasNormals(false), HasTangents(false),
		HasTextureUvs(false), HasVertexColors(false), HasJoints(0), 
		IndexMode(2), Topology(0)
	{}

	inline void SetMaterial(Pu::uint32 material)
	{
		WriteMaterialIndex = true;
		Material = material;
	}

	inline Pu::uint16 GetFlags(void) const
	{
		Pu::uint16 result = 0;

		if (WriteMaterialIndex) result |= 0x01;
		if (HasNormals) result |= 0x02;
		if (HasTangents) result |= 0x04;
		if (HasTextureUvs) result |= 0x08;
		if (HasVertexColors)result |= 0x10;
		result |= (HasJoints & 0x03) << 0x05;
		result |= (IndexMode & 0x03) << 0x07;
		result |= (Topology & 0x07) << 0x09;

		return result;
	}
};

struct pum_frame
{
	float Time;
	Pu::Vector3 Translation;
	Pu::Quaternion Rotation;
	Pu::Vector3 Scale;
	Pu::AABB Bounds;
};

struct pum_sequency
{
	Pu::uint32 Node;
	Pu::vector<pum_frame> Frames;
};

struct pum_animation
{
	Pu::ustring Identifier;
	Pu::vector<pum_sequency> Sequences;
	Pu::vector<Pu::uint32> Frames;

	int InterpolationMode;
	bool Looping;
	bool Reverse;
	bool Type;
	bool ShouldBake;

	float Arg1;
	float Arg2;
	float Duration;

	pum_animation(void)
		: InterpolationMode(0), Looping(false), Reverse(false), Type(false)
	{}

	inline Pu::byte GetFlags(void) const
	{
		Pu::byte result = (InterpolationMode & 0x7);
		if (Looping) result |= 0x08;
		if (Reverse) result |= 0x10;
		if (Type) result |= 0x20;
		if (ShouldBake) result |= 0x40;
		return result;
	}
};

struct pum_joint
{
	Pu::uint32 Node;
	Pu::Matrix IBind;
};

struct pum_skeleton
{
	Pu::ustring Identifier;
	Pu::uint32 Root;
	Pu::vector<pum_joint> Joints;
};

struct pum_material
{
	Pu::ustring Identifier;
	Pu::Color DiffuseFactor;
	Pu::Color SpecularFactor;
	Pu::Color EmissiveFactor;
	float Glossiness;
	float SpecularPower;
	float EmissiveInternsity;
	float Metalness;

	bool IsFinalized;
	bool DoubleSided;
	char AlphaMode;
	bool HasDiffuseTexture;
	bool HasSpecularGlossTexture;
	bool HasNormalTexture;
	bool HasOcclusionTexture;
	bool HasEmissiveTexture;

	float AlphaThreshold;
	Pu::uint32 DiffuseTexture;
	Pu::uint32 SpecGlossTexture;
	Pu::uint32 NormalTexture;
	Pu::uint32 OcclusionTexture;
	Pu::uint32 EmissiveTexture;

	pum_material(void)
		: Glossiness(0.0f), SpecularPower(0.0f), EmissiveInternsity(0.0f), DoubleSided(false), 
		AlphaMode(0), HasDiffuseTexture(false), HasSpecularGlossTexture(false), Metalness(0.0f),
		HasNormalTexture(false), HasOcclusionTexture(false), HasEmissiveTexture(false), IsFinalized(true)
	{}

	inline void SetDiffuseTexture(Pu::uint32 diffuseTexture)
	{
		HasDiffuseTexture = true;
		DiffuseTexture = diffuseTexture;
	}

	inline void SetSpecGlossTexture(Pu::uint32 specGlossTexture)
	{
		HasSpecularGlossTexture = true;
		SpecGlossTexture = specGlossTexture;
	}

	inline void SetNormalTexture(Pu::uint32 normalTexture)
	{
		HasNormalTexture = true;
		NormalTexture = normalTexture;
	}

	inline void SetOcclusionTexture(Pu::uint32 occlusionTexture)
	{
		HasOcclusionTexture = true;
		OcclusionTexture = occlusionTexture;
	}

	inline void SetEmissiveTexture(Pu::uint32 emissiveTexture)
	{
		HasEmissiveTexture = true;
		EmissiveTexture = emissiveTexture;
	}

	inline Pu::byte GetFlags(void) const
	{
		Pu::byte result = 0;

		if (DoubleSided) result |= 1;
		result |= (AlphaMode & 0x03) << 1;
		if (HasDiffuseTexture) result |= 0x08;
		if (HasSpecularGlossTexture) result |= 0x10;
		if (HasNormalTexture) result |= 0x20;
		if (HasOcclusionTexture) result |= 0x40;
		if (HasEmissiveTexture) result |= 0x80;

		return result;
	}
};

struct pum_texture
{
	Pu::ustring Identifier;

	bool UsesLinearMagnification;
	bool UsesLinaerMinification;
	bool UsesLinearMipmapMode;
	char AddressModeU;
	char AddressModeV;
	int ConversionCount;

	pum_texture(void)
		: UsesLinearMagnification(false), UsesLinaerMinification(false),
		UsesLinearMipmapMode(false), AddressModeU(0), AddressModeV(0), ConversionCount(-1)
	{}

	inline Pu::byte GetFlags(void) const
	{
		Pu::byte result = 0;

		if (UsesLinearMagnification) result |= 0x01;
		if (UsesLinaerMinification) result |= 0x02;
		if (UsesLinearMipmapMode) result |= 0x04;
		result |= (AddressModeU & 0x03) << 0x03;
		result |= (AddressModeV & 0x03) << 0x05;

		return result;
	}
};

struct PumIntermediate
{
	Pu::vector<pum_node> Nodes;
	Pu::vector<pum_mesh> Geometry;
	Pu::vector<pum_animation> Animations;
	Pu::vector<pum_skeleton> Skeletons;
	Pu::vector<pum_material> Materials;
	Pu::vector<pum_texture> Textures;
	Pu::BinaryWriter Data{ 0, Pu::Endian::Little };
};