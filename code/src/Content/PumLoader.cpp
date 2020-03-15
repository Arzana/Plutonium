#include "Content/PumLoader.h"
#include "Streams/FileReader.h"

Pu::PumNode::PumNode(void)
	: HasMesh(false), HasSkin(false), HasTranslation(false),
	HasRotation(false), HasScale(false), Mesh(0), Skin(0)
{}

Pu::PumNode::PumNode(BinaryReader & reader)
	: Mesh(0), Skin(0)
{
	const uint32 childCount = reader.ReadUInt32();

	Children.reserve(childCount);
	for (uint32 i = 0; i < childCount; i++) Children.emplace_back(reader.ReadUInt32());

	const byte flags = reader.ReadByte();
	HasMesh = flags & 0x1;
	HasSkin = flags & 0x2;
	HasTranslation = flags & 0x4;
	HasRotation = flags & 0x8;
	HasScale = flags & 0x10;

	if (HasMesh) Mesh = reader.ReadUInt32();
	if (HasSkin) Skin = reader.ReadUInt32();
	if (HasTranslation) Translation = reader.ReadVector3();
	if (HasRotation) Rotation = reader.ReadPackedQuaternion();
	if (HasScale) Scale = reader.ReadVector3();
}

Pu::Matrix Pu::PumNode::GetTransform(void) const
{
	Matrix result;
	if (HasTranslation) result *= Matrix::CreateTranslation(Translation);
	if (HasRotation) result *= Matrix::CreateRotation(Rotation);
	if (HasScale) result *= Matrix::CreateScalar(Scale);
	return result;
}

Pu::PumMesh::PumMesh(void)
	: HasMaterial(false), HasNormals(false), HasTangents(false), HasTextureCoordinates(false),
	HasColors(false), JointType(PumJointType::None), Topology(PrimitiveTopology::PointList),
	Material(0), VertexViewStart(0), VertexViewSize(0)
{}

Pu::PumMesh::PumMesh(BinaryReader & reader)
	: Material(0), IndexViewStart(0), IndexViewSize(0)
{
	Identifier = reader.ReadUString();

	const uint16 flags = reader.ReadUInt16();
	HasMaterial = flags & 0x1;
	HasNormals = flags & 0x2;
	HasTangents = flags & 0x4;
	HasTextureCoordinates = flags & 0x8;
	HasColors = flags & 0x10;
	JointType = _CrtInt2Enum<PumJointType>((flags & 0x60) >> 0x5);
	IndexType = _CrtInt2Enum<PumIndexType>((flags & 0x180) >> 0x7);
	Topology = _CrtInt2Enum<PrimitiveTopology>((flags & 0xE00) >> 0x9);

	Bounds.LowerBound = reader.ReadVector3();
	Bounds.UpperBound = reader.ReadVector3();

	VertexViewStart = reader.ReadUInt64();
	VertexViewSize = reader.ReadUInt64();

	if (HasMaterial) Material = reader.ReadUInt32();
	if (IndexType != PumIndexType::None)
	{
		IndexViewStart = reader.ReadUInt64();
		IndexViewSize = reader.ReadUInt64();
	}
}

Pu::PumFrame::PumFrame(void)
	: TimeStamp(0.0f)
{}

Pu::PumFrame::PumFrame(BinaryReader & reader)
{
	TimeStamp = reader.ReadSingle();
	Translation = reader.ReadVector3();
	Rotation = reader.ReadPackedQuaternion();
	Scale = reader.ReadVector3();
	Bounds.LowerBound = reader.ReadVector3();
	Bounds.UpperBound = reader.ReadVector3();
}

Pu::PumSequence::PumSequence(void)
	: Node(0)
{}

Pu::PumSequence::PumSequence(BinaryReader & reader)
{
	Node = reader.ReadUInt32();

	const uint32 frameCount = reader.ReadUInt32();
	Frames.reserve(frameCount);
	for (uint32 i = 0; i < frameCount; i++) Frames.emplace_back(reader);
}

Pu::PumAnimation::PumAnimation(void)
	: Interpolation(PumInterpolationType::None), ShouldLoop(false), PlayInReverse(false),
	IsMorphAnimation(false), ShouldBake(false), Duration(0.0f)
{}

Pu::PumAnimation::PumAnimation(BinaryReader & reader)
	: Arg1(0.0f), Arg2(0.0f), Duration(0.0f)
{
	Identifier = reader.ReadUString();

	const byte flags = reader.ReadByte();
	Interpolation = _CrtInt2Enum<PumInterpolationType>(flags & 0x7);
	ShouldLoop = flags & 0x8;
	PlayInReverse = flags & 0x10;
	IsMorphAnimation = flags & 0x20;
	ShouldBake = flags & 0x40;

	if (Interpolation == PumInterpolationType::Cubic || Interpolation == PumInterpolationType::Spring)
	{
		Arg1 = reader.ReadSingle();
		Arg2 = reader.ReadSingle();
	}

	const uint32 count = reader.ReadUInt32();
	if (IsMorphAnimation)
	{
		Frames.reserve(count);
		for (uint32 i = 0; i < count; i++) Frames.emplace_back(reader.ReadUInt32());
		Duration = reader.ReadSingle();
	}
	else
	{
		Sequences.reserve(count);
		for (uint32 i = 0; i < count; i++)
		{
			PumSequence sequence(reader);
			for (const PumFrame &frame : sequence.Frames) Duration = max(Duration, frame.TimeStamp);
			Sequences.emplace_back(std::move(sequence));
		}
	}
}

Pu::PumJoint::PumJoint(void)
	: Node(0)
{}

Pu::PumJoint::PumJoint(BinaryReader & reader)
{
	Node = reader.ReadUInt32();
	IBind = reader.ReadMatrix();
}

Pu::PumSkeleton::PumSkeleton(void)
	: Root(0)
{}

Pu::PumSkeleton::PumSkeleton(BinaryReader & reader)
{
	Identifier = reader.ReadUString();
	Root = reader.ReadUInt32();

	const uint32 jointCount = reader.ReadUInt32();
	Joints.reserve(jointCount);
	for (uint32 i = 0; i < jointCount; i++) Joints.emplace_back(reader);
}

Pu::PumMaterial::PumMaterial(void)
	: DoubleSided(false), AlphaMode(PumAlphaMode::Opaque), HasDiffuseTexture(false),
	HasSpecGlossTexture(false), HasNormalTexture(false), HasOcclusionTexture(false),
	HasEmissiveTexture(false), Glossiness(0.0f), SpecularPower(0.0f), EmissiveIntensity(0.0f)
{}

Pu::PumMaterial::PumMaterial(BinaryReader & reader)
	: AlphaTheshold(0.0f), DiffuseTexture(0), SpecGlossTexture(0),
	NormalTexture(0), OcclusionTexture(0), EmissiveTexture(0)
{
	Identifier = reader.ReadUString();

	const byte flags = reader.ReadByte();
	DoubleSided = flags & 0x1;
	AlphaMode = _CrtInt2Enum<PumAlphaMode>((flags & 0x6) >> 1);
	HasDiffuseTexture = flags & 0x8;
	HasSpecGlossTexture = flags & 0x10;
	HasNormalTexture = flags & 0x20;
	HasOcclusionTexture = flags & 0x40;
	HasEmissiveTexture = flags & 0x80;

	DiffuseFactor.Packed = reader.ReadUInt32();
	SpecularFactor.Packed = reader.ReadUInt32();
	EmissiveFactor.Packed = reader.ReadUInt32();
	Glossiness = reader.ReadSingle();
	SpecularPower = reader.ReadSingle();
	EmissiveIntensity = reader.ReadSingle();

	if (AlphaMode == PumAlphaMode::Mask) AlphaTheshold = reader.ReadSingle();
	if (HasDiffuseTexture) DiffuseTexture = reader.ReadUInt32();
	if (HasSpecGlossTexture) SpecGlossTexture = reader.ReadUInt32();
	if (HasNormalTexture) NormalTexture = reader.ReadUInt32();
	if (HasOcclusionTexture) OcclusionTexture = reader.ReadUInt32();
	if (HasEmissiveTexture) EmissiveTexture = reader.ReadUInt32();
}

Pu::PumTexture::PumTexture(void)
	: Magnification(Filter::Nearest), Minification(Filter::Nearest), MipMap(SamplerMipmapMode::Nearest),
	AddressModeU(SamplerAddressMode::Repeat), AddressModeV(SamplerAddressMode::Repeat), IsSRGB(false)
{}

Pu::PumTexture::PumTexture(BinaryReader & reader)
{
	Path = reader.ReadUString();

	const byte flags = reader.ReadByte();
	Magnification = _CrtInt2Enum<Filter>(flags & 0x1);
	Minification = _CrtInt2Enum<Filter>((flags & 0x2) >> 1);
	MipMap = _CrtInt2Enum<SamplerMipmapMode>((flags & 0x4) >> 2);
	IsSRGB = flags & 0x8;
	AddressModeU = _CrtInt2Enum<SamplerAddressMode>((flags & 0x30) >> 4);
	AddressModeV = _CrtInt2Enum<SamplerAddressMode>((flags & 0xC0) >> 6);
}

Pu::SamplerCreateInfo Pu::PumTexture::GetSamplerCreateInfo(void) const
{
	SamplerCreateInfo info;
	info.MagFilter = Magnification;
	info.MinFilter = Minification;
	info.MipmapMode = MipMap;
	info.AddressModeU = AddressModeU;
	info.AddressModeV = AddressModeV;
	info.AnisotropyEnable = true;
	info.MaxAnisotropy = 4.0f;
	info.MaxLoD = static_cast<float>(DefaultMipLevels);
	return info;
}

Pu::PuMData::PuMData(void)
	: Version(0), Buffer(nullptr)
{}

Pu::PuMData::PuMData(LogicalDevice & device, BinaryReader & reader)
{
	if (reader.ReadUInt32() != Stream::GetMagicNum("PUM0"))
	{
		Raise("magic number mismatch");
		return;
	}

	Version = reader.ReadUInt32();
	Identifier = reader.ReadUString();

	const uint32 nodeCount = reader.ReadUInt32();
	const uint32 meshCount = reader.ReadUInt32();
	const uint32 animationCount = reader.ReadUInt32();
	const uint32 skeletonCount = reader.ReadUInt32();
	const uint32 materialCount = reader.ReadUInt32();
	const uint32 textureCount = reader.ReadUInt32();

	const uint64 nodeOffset = reader.ReadUInt64();
	const uint64 meshOffset = reader.ReadUInt64();
	const uint64 animationOffset = reader.ReadUInt64();
	const uint64 skeletonOffset = reader.ReadUInt64();
	const uint64 materialOffset = reader.ReadUInt64();
	const uint64 textureOffset = reader.ReadUInt64();
	const uint64 bufferOffset = reader.ReadUInt64();
	const uint64 bufferSize = reader.ReadUInt64();

	Nodes.reserve(nodeCount);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(nodeOffset));
	for (uint32 i = 0; i < nodeCount; i++) Nodes.emplace_back(reader);

	Geometry.reserve(meshCount);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(meshOffset));
	for (uint32 i = 0; i < meshCount; i++) Geometry.emplace_back(reader);

	Animations.reserve(animationCount);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(animationOffset));
	for (uint32 i = 0; i < animationCount; i++) Animations.emplace_back(reader);

	Skeletons.reserve(skeletonCount);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(skeletonOffset));
	for (uint32 i = 0; i < skeletonCount; i++) Skeletons.emplace_back(reader);

	Materials.reserve(materialCount);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(materialOffset));
	for (uint32 i = 0; i < materialCount; i++) Materials.emplace_back(reader);

	Textures.reserve(textureCount);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(textureOffset));
	for (uint32 i = 0; i < textureCount; i++) Textures.emplace_back(reader);

	Buffer = new StagingBuffer(device, bufferSize);
	Buffer->Load(reinterpret_cast<const byte*>(reader.GetData()) + bufferOffset);
}

Pu::PuMData Pu::PuMData::TexturesOnly(const wstring & path)
{
	FileReader reader{ path };

	/* Read the header information. */
	PuMData result;
	if (!SetHeader(result, reader)) return result;

	/* We can skip the first 5 counts. */
	reader.Seek(SeekOrigin::Current, sizeof(uint32) * 5);

	/* Get the amount of textures. */
	uint32 textureCount;
	reader.Read(textureCount);
	result.Textures.reserve(textureCount);

	/* The first 5 offsets can be skipped. */
	reader.Seek(SeekOrigin::Current, sizeof(uint64) * 5);

	/* Read the texture offset and seek to it. */
	uint64 offset, end;
	reader.Read(offset);
	reader.Read(end);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(offset));

	/* Setup a binary reader for the texture part. */
	const uint64 size = end - offset;
	byte *data = reinterpret_cast<byte*>(malloc(size));
	reader.Read(data, 0, size);
	BinaryReader binary{ data, size, Endian::Little };

	/* Read the texture data. */
	for (uint32 i = 0; i < textureCount; i++) result.Textures.emplace_back(binary);
	free(data);
	return result;
}

Pu::PuMData Pu::PuMData::MeshesOnly(LogicalDevice & device, const wstring & path)
{
	FileReader reader{ path };

	/* Read the header information. */
	PuMData result;
	if (!SetHeader(result, reader)) return result;

	/* We can skip the node count. */
	reader.Seek(SeekOrigin::Current, sizeof(uint32));

	/* Get the amount of meshes.. */
	uint32 meshCount;
	reader.Read(meshCount);
	result.Geometry.reserve(meshCount);

	/* The remaining counts can be skipped and the first offset. */
	reader.Seek(SeekOrigin::Current, (sizeof(uint32) << 2) + sizeof(uint64));

	/* Read the mesh offset and the mesh end. */
	uint64 offset, end;
	reader.Read(offset);
	reader.Read(end);

	/* The remaining offsets can be skipped. */
	reader.Seek(SeekOrigin::Current, (sizeof(uint64) * 3));

	/* Read the GPU buffer offset and size. */
	uint64 bufferOffset, bufferSize;
	reader.Read(bufferOffset);
	reader.Read(bufferSize);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(offset));

	/* Setup a binary reader for the mesh part. */
	const uint64 size = end - offset;
	byte *data = reinterpret_cast<byte*>(malloc(size));
	reader.Read(data, 0, size);
	BinaryReader binary{ data, size, Endian::Little };

	/* Read the geometry data. */
	for (uint32 i = 0; i < meshCount; i++) result.Geometry.emplace_back(binary);
	free(data);

	/* Read the GPU buffer data. */
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(bufferOffset));
	result.Buffer = new StagingBuffer(device, bufferSize);
	result.Buffer->BeginMemoryTransfer();
	reader.Read(reinterpret_cast<byte*>(result.Buffer->GetHostMemory()), 0, bufferSize);
	result.Buffer->EndMemoryTransfer();

	return result;
}

Pu::PuMData Pu::PuMData::MaterialsOnly(const wstring & path)
{
	FileReader reader{ path };

	/* Read the header information. */
	PuMData result;
	if (!SetHeader(result, reader)) return result;

	/* We can skip the first 4 counts. */
	reader.Seek(SeekOrigin::Current, (sizeof(uint32) << 2));

	/* Get the amount of materials. */
	uint32 materialCount;
	reader.Read(materialCount);
	result.Materials.reserve(materialCount);

	/* The first 4 offsets can be skipped and the texture count. */
	reader.Seek(SeekOrigin::Current, sizeof(uint32) + (sizeof(uint64) << 2));

	/* Read the material offset and seek to it. */
	uint64 offset, end;
	reader.Read(offset);
	reader.Read(end);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(offset));

	/* Setup a binary reader for the material part. */
	const uint64 size = end - offset;
	byte *data = reinterpret_cast<byte*>(malloc(size));
	reader.Read(data, 0, size);
	BinaryReader binary{ data, size, Endian::Little };

	/* Read the material data. */
	for (uint32 i = 0; i < materialCount; i++) result.Materials.emplace_back(binary);
	free(data);
	return result;
}

bool Pu::PuMData::SetHeader(PuMData & result, FileReader & reader)
{
	/* Check the magic number. */
	uint32 value;
	if (!reader.Read(value))
	{
		Raise("not enough bytes for magic number");
		return false;
	}

	if (value != Stream::GetMagicNum("PUM0"))
	{
		Raise("magic number mismatch");
		return false;
	}

	/* Set the version. */
	if (!reader.Read(value))
	{
		Raise("not enough bytes for version");
		return false;
	}

	result.Version = value;

	/* Set the identifier. */
	if (!reader.Read(value))
	{
		Raise("not engouh bytes for identifier length");
		return false;
	}

	char32 c;
	result.Identifier.reserve(value);
	for (uint32 i = 0; i < value; i++)
	{
		if (!reader.Read(c))
		{
			Raise("corrupt identifier");
			return false;
		}

		result.Identifier += c;
	}

	return true;
}

void Pu::PuMData::Raise(const char * reason)
{
	Log::Error("Unable to load Plutonium model (%s)!", reason);
}