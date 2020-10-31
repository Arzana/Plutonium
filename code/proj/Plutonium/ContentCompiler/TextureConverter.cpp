#include "TextureConverter.h"
#include <Streams/FileWriter.h>
#include <Streams/FileReader.h>
#include <Content/AssetSaver.h>
#include <Content/AssetFetcher.h>
#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Framebuffer.h>
#include <Graphics/Vulkan/CommandPool.h>
#include <Core/Diagnostics/Profiler.h>
#include <Core/Threading/PuThread.h>
#include <Core/Threading/Tasks/Scheduler.h>

using namespace Pu;

/* Defines the uniform block used by the conversion shader. */
class ConverterUniformBlock
	: public DescriptorSet
{
public:
	ConverterUniformBlock(DescriptorPool &pool, const DescriptorSetLayout &layout, float metal, float roughness, Color factor)
		: DescriptorSet(pool, 0, layout), metalFactor(metal),
		roughnessFactor(roughness), albedoFactor(factor.ToVector4()),
		albedo(&GetDescriptor(0, "Albedo")),
		metalRough(&GetDescriptor(0, "MetalRoughness"))
	{}

	void SetAlbedo(const Texture2D &map)
	{
		Write(*albedo, map);
	}

	void SetMetalRough(const Texture2D &map)
	{
		Write(*metalRough, map);
	}

protected:
	virtual void Stage(byte *dest) override
	{
		Copy(dest, &albedoFactor);
		Copy(dest + sizeof(Vector4), &roughnessFactor);
		Copy(dest + sizeof(Vector4) + sizeof(float), &metalFactor);
	}

private:
	float metalFactor, roughnessFactor;
	Vector4 albedoFactor;
	const Descriptor *albedo, *metalRough;
};

#pragma region SHADERS
/*
#version 460 core

layout (location = 0) out vec2 Uv;

void main()
{
	Uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(Uv * 2.0f - 1.0f, 0.0f, 1.0f);
}
*/
static const uint32 VERTEX_SHADER[] =
{
	0x07230203, 0x00010000, 0x00080007, 0x0000002b, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
	0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
	0x0008000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000c, 0x0000001d,
	0x00030003, 0x00000002, 0x000001cc, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00030005,
	0x00000009, 0x00007655, 0x00060005, 0x0000000c, 0x565f6c67, 0x65747265, 0x646e4978, 0x00007865,
	0x00060005, 0x0000001b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000001b,
	0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000001b, 0x00000001, 0x505f6c67,
	0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000001b, 0x00000002, 0x435f6c67, 0x4470696c,
	0x61747369, 0x0065636e, 0x00070006, 0x0000001b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
	0x0065636e, 0x00030005, 0x0000001d, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000,
	0x00040047, 0x0000000c, 0x0000000b, 0x0000002a, 0x00050048, 0x0000001b, 0x00000000, 0x0000000b,
	0x00000000, 0x00050048, 0x0000001b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000001b,
	0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000001b, 0x00000003, 0x0000000b, 0x00000004,
	0x00030047, 0x0000001b, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
	0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000002, 0x00040020,
	0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040015,
	0x0000000a, 0x00000020, 0x00000001, 0x00040020, 0x0000000b, 0x00000001, 0x0000000a, 0x0004003b,
	0x0000000b, 0x0000000c, 0x00000001, 0x0004002b, 0x0000000a, 0x0000000e, 0x00000001, 0x0004002b,
	0x0000000a, 0x00000010, 0x00000002, 0x00040017, 0x00000017, 0x00000006, 0x00000004, 0x00040015,
	0x00000018, 0x00000020, 0x00000000, 0x0004002b, 0x00000018, 0x00000019, 0x00000001, 0x0004001c,
	0x0000001a, 0x00000006, 0x00000019, 0x0006001e, 0x0000001b, 0x00000017, 0x00000006, 0x0000001a,
	0x0000001a, 0x00040020, 0x0000001c, 0x00000003, 0x0000001b, 0x0004003b, 0x0000001c, 0x0000001d,
	0x00000003, 0x0004002b, 0x0000000a, 0x0000001e, 0x00000000, 0x0004002b, 0x00000006, 0x00000020,
	0x40000000, 0x0004002b, 0x00000006, 0x00000022, 0x3f800000, 0x0004002b, 0x00000006, 0x00000025,
	0x00000000, 0x00040020, 0x00000029, 0x00000003, 0x00000017, 0x00050036, 0x00000002, 0x00000004,
	0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c,
	0x000500c4, 0x0000000a, 0x0000000f, 0x0000000d, 0x0000000e, 0x000500c7, 0x0000000a, 0x00000011,
	0x0000000f, 0x00000010, 0x0004006f, 0x00000006, 0x00000012, 0x00000011, 0x0004003d, 0x0000000a,
	0x00000013, 0x0000000c, 0x000500c7, 0x0000000a, 0x00000014, 0x00000013, 0x00000010, 0x0004006f,
	0x00000006, 0x00000015, 0x00000014, 0x00050050, 0x00000007, 0x00000016, 0x00000012, 0x00000015,
	0x0003003e, 0x00000009, 0x00000016, 0x0004003d, 0x00000007, 0x0000001f, 0x00000009, 0x0005008e,
	0x00000007, 0x00000021, 0x0000001f, 0x00000020, 0x00050050, 0x00000007, 0x00000023, 0x00000022,
	0x00000022, 0x00050083, 0x00000007, 0x00000024, 0x00000021, 0x00000023, 0x00050051, 0x00000006,
	0x00000026, 0x00000024, 0x00000000, 0x00050051, 0x00000006, 0x00000027, 0x00000024, 0x00000001,
	0x00070050, 0x00000017, 0x00000028, 0x00000026, 0x00000027, 0x00000025, 0x00000022, 0x00050041,
	0x00000029, 0x0000002a, 0x0000001d, 0x0000001e, 0x0003003e, 0x0000002a, 0x00000028, 0x000100fd,
	0x00010038
};

/*
#version 460 core

layout (binding = 0) uniform sampler2D Albedo;
layout (binding = 1) uniform sampler2D MetalRoughness;

layout (binding = 2) uniform Properties
{
	vec4 AlbedoFactor;
	float RoughnessFactor;
	float MetalFactor;
};

layout (location = 0) in vec2 Uv;

layout (location = 0) out vec4 Diffuse;
layout (location = 1) out vec4 SpecularGloss;

void main()
{
	// Get the raw values from the sampelrs.
	const vec4 albedo = texture(Albedo, Uv) * AlbedoFactor;
	const vec3 metalRough = texture(MetalRoughness, Uv).rgb;
	const float metal = metalRough.b * MetalFactor;
	const float roughness = metalRough.g * RoughnessFactor;

	// Defines constants used for parsing.
	const vec3 dieletric = vec3(0.04f, 0.04f, 0.04f);
	const vec4 black = vec4(0.0f, 0.0f, 0.0f, albedo.a);

	// Convert from metal/roughness to specular/glossiness.
	Diffuse = mix(albedo, black, metal);
	SpecularGloss.rgb = mix(dieletric, albedo.rgb, metal);
	SpecularGloss.a = 1.0f - roughness;
}
*/
static const uint32 FRAGMENT_SHADER[] =
{
	0x07230203,	0x00010000,	0x00080007,	0x00000054,	0x00000000,	0x00020011, 0x00000001, 0x0006000b,
	0x00000001,	0x4c534c47,	0x6474732e,	0x3035342e,	0x00000000,	0x0003000e, 0x00000000, 0x00000001,
	0x0008000f,	0x00000004,	0x00000004,	0x6e69616d,	0x00000000,	0x00000011, 0x0000003f, 0x00000045,
	0x00030010,	0x00000004,	0x00000007,	0x00030003,	0x00000002,	0x000001cc, 0x00040005, 0x00000004,
	0x6e69616d,	0x00000000,	0x00040005,	0x00000009,	0x65626c61,	0x00006f64, 0x00040005, 0x0000000d,
	0x65626c41,	0x00006f64,	0x00030005,	0x00000011,	0x00007655,	0x00050005, 0x00000014, 0x706f7250,
	0x69747265,	0x00007365,	0x00070006,	0x00000014,	0x00000000,	0x65626c41, 0x61466f64, 0x726f7463,
	0x00000000,	0x00070006,	0x00000014,	0x00000001,	0x67756f52,	0x73656e68, 0x63614673, 0x00726f74,
	0x00060006,	0x00000014,	0x00000002,	0x6174654d,	0x6361466c,	0x00726f74, 0x00030005, 0x00000016,
	0x00000000,	0x00050005,	0x0000001f,	0x6174656d,	0x756f526c,	0x00006867, 0x00060005, 0x00000020,
	0x6174654d,	0x756f526c,	0x656e6867,	0x00007373,	0x00040005,	0x00000026, 0x6174656d, 0x0000006c,
	0x00050005,	0x00000030,	0x67756f72,	0x73656e68,	0x00000073,	0x00040005, 0x00000038, 0x63616c62,
	0x0000006b,	0x00040005,	0x0000003f,	0x66666944,	0x00657375,	0x00060005, 0x00000045, 0x63657053,
	0x72616c75,	0x736f6c47,	0x00000073,	0x00040047,	0x0000000d,	0x00000022, 0x00000000, 0x00040047,
	0x0000000d,	0x00000021,	0x00000000,	0x00040047,	0x00000011,	0x0000001e, 0x00000000, 0x00050048,
	0x00000014,	0x00000000,	0x00000023,	0x00000000,	0x00050048,	0x00000014, 0x00000001, 0x00000023,
	0x00000010,	0x00050048,	0x00000014,	0x00000002,	0x00000023,	0x00000014, 0x00030047, 0x00000014,
	0x00000002,	0x00040047,	0x00000016,	0x00000022,	0x00000000,	0x00040047, 0x00000016, 0x00000021,
	0x00000002,	0x00040047,	0x00000020,	0x00000022,	0x00000000,	0x00040047, 0x00000020, 0x00000021,
	0x00000001,	0x00040047,	0x0000003f,	0x0000001e,	0x00000000,	0x00040047, 0x00000045, 0x0000001e,
	0x00000001,	0x00020013,	0x00000002,	0x00030021,	0x00000003,	0x00000002, 0x00030016, 0x00000006,
	0x00000020,	0x00040017,	0x00000007,	0x00000006,	0x00000004,	0x00040020, 0x00000008, 0x00000007,
	0x00000007,	0x00090019,	0x0000000a,	0x00000006,	0x00000001,	0x00000000, 0x00000000, 0x00000000,
	0x00000001,	0x00000000,	0x0003001b,	0x0000000b,	0x0000000a,	0x00040020, 0x0000000c, 0x00000000,
	0x0000000b,	0x0004003b,	0x0000000c,	0x0000000d,	0x00000000,	0x00040017, 0x0000000f, 0x00000006,
	0x00000002,	0x00040020,	0x00000010,	0x00000001,	0x0000000f,	0x0004003b, 0x00000010, 0x00000011,
	0x00000001,	0x0005001e,	0x00000014,	0x00000007,	0x00000006,	0x00000006, 0x00040020, 0x00000015,
	0x00000002,	0x00000014,	0x0004003b,	0x00000015,	0x00000016,	0x00000002, 0x00040015, 0x00000017,
	0x00000020,	0x00000001,	0x0004002b,	0x00000017,	0x00000018,	0x00000000, 0x00040020, 0x00000019,
	0x00000002,	0x00000007,	0x00040017,	0x0000001d,	0x00000006,	0x00000003, 0x00040020, 0x0000001e,
	0x00000007,	0x0000001d,	0x0004003b,	0x0000000c,	0x00000020,	0x00000000, 0x00040020, 0x00000025,
	0x00000007,	0x00000006,	0x00040015,	0x00000027,	0x00000020,	0x00000000, 0x0004002b, 0x00000027,
	0x00000028,	0x00000002,	0x0004002b,	0x00000017,	0x0000002b,	0x00000002, 0x00040020, 0x0000002c,
	0x00000002,	0x00000006,	0x0004002b,	0x00000027,	0x00000031,	0x00000001, 0x0004002b, 0x00000017,
	0x00000034,	0x00000001,	0x0004002b,	0x00000006,	0x00000039,	0x00000000, 0x0004002b, 0x00000027,
	0x0000003a,	0x00000003,	0x00040020,	0x0000003e,	0x00000003,	0x00000007, 0x0004003b, 0x0000003e,
	0x0000003f,	0x00000003,	0x0004003b,	0x0000003e,	0x00000045,	0x00000003, 0x0004002b, 0x00000006,
	0x00000046,	0x3d23d70a,	0x0006002c,	0x0000001d,	0x00000047,	0x00000046, 0x00000046, 0x00000046,
	0x0004002b,	0x00000006,	0x0000004f,	0x3f800000,	0x00040020,	0x00000052, 0x00000003, 0x00000006,
	0x00050036,	0x00000002,	0x00000004,	0x00000000,	0x00000003,	0x000200f8, 0x00000005, 0x0004003b,
	0x00000008,	0x00000009,	0x00000007,	0x0004003b,	0x0000001e,	0x0000001f, 0x00000007, 0x0004003b,
	0x00000025,	0x00000026,	0x00000007,	0x0004003b,	0x00000025,	0x00000030, 0x00000007, 0x0004003b,
	0x00000008,	0x00000038,	0x00000007,	0x0004003d,	0x0000000b,	0x0000000e, 0x0000000d, 0x0004003d,
	0x0000000f,	0x00000012,	0x00000011,	0x00050057,	0x00000007,	0x00000013, 0x0000000e, 0x00000012,
	0x00050041,	0x00000019,	0x0000001a,	0x00000016,	0x00000018,	0x0004003d, 0x00000007, 0x0000001b,
	0x0000001a,	0x00050085,	0x00000007,	0x0000001c,	0x00000013,	0x0000001b, 0x0003003e, 0x00000009,
	0x0000001c,	0x0004003d,	0x0000000b,	0x00000021,	0x00000020,	0x0004003d, 0x0000000f, 0x00000022,
	0x00000011,	0x00050057,	0x00000007,	0x00000023,	0x00000021,	0x00000022, 0x0008004f, 0x0000001d,
	0x00000024,	0x00000023,	0x00000023,	0x00000000,	0x00000001,	0x00000002, 0x0003003e, 0x0000001f,
	0x00000024,	0x00050041,	0x00000025,	0x00000029,	0x0000001f,	0x00000028, 0x0004003d, 0x00000006,
	0x0000002a,	0x00000029,	0x00050041,	0x0000002c,	0x0000002d,	0x00000016, 0x0000002b, 0x0004003d,
	0x00000006,	0x0000002e,	0x0000002d,	0x00050085,	0x00000006,	0x0000002f, 0x0000002a, 0x0000002e,
	0x0003003e,	0x00000026,	0x0000002f,	0x00050041,	0x00000025,	0x00000032, 0x0000001f, 0x00000031,
	0x0004003d,	0x00000006,	0x00000033,	0x00000032,	0x00050041,	0x0000002c, 0x00000035, 0x00000016,
	0x00000034,	0x0004003d,	0x00000006,	0x00000036,	0x00000035,	0x00050085, 0x00000006, 0x00000037,
	0x00000033,	0x00000036,	0x0003003e,	0x00000030,	0x00000037,	0x00050041, 0x00000025, 0x0000003b,
	0x00000009,	0x0000003a,	0x0004003d,	0x00000006,	0x0000003c,	0x0000003b, 0x00070050, 0x00000007,
	0x0000003d,	0x00000039,	0x00000039,	0x00000039,	0x0000003c,	0x0003003e, 0x00000038, 0x0000003d,
	0x0004003d,	0x00000007,	0x00000040,	0x00000009,	0x0004003d,	0x00000007, 0x00000041, 0x00000038,
	0x0004003d,	0x00000006,	0x00000042,	0x00000026,	0x00070050,	0x00000007, 0x00000043, 0x00000042,
	0x00000042,	0x00000042,	0x00000042,	0x0008000c,	0x00000007,	0x00000044, 0x00000001, 0x0000002e,
	0x00000040,	0x00000041,	0x00000043,	0x0003003e,	0x0000003f,	0x00000044, 0x0004003d, 0x00000007,
	0x00000048,	0x00000009,	0x0008004f,	0x0000001d,	0x00000049,	0x00000048, 0x00000048, 0x00000000,
	0x00000001,	0x00000002,	0x0004003d,	0x00000006,	0x0000004a,	0x00000026, 0x00060050, 0x0000001d,
	0x0000004b,	0x0000004a,	0x0000004a,	0x0000004a,	0x0008000c,	0x0000001d, 0x0000004c, 0x00000001,
	0x0000002e,	0x00000047,	0x00000049,	0x0000004b,	0x0004003d,	0x00000007, 0x0000004d, 0x00000045,
	0x0009004f,	0x00000007,	0x0000004e,	0x0000004d,	0x0000004c,	0x00000004, 0x00000005, 0x00000006,
	0x00000003,	0x0003003e,	0x00000045,	0x0000004e,	0x0004003d,	0x00000006, 0x00000050, 0x00000030,
	0x00050083,	0x00000006,	0x00000051,	0x0000004f,	0x00000050,	0x00050041, 0x00000052, 0x00000053,
	0x00000045,	0x0000003a,	0x0003003e,	0x00000053,	0x00000051,	0x000100fd, 0x00010038
};
#pragma endregion

constexpr uint32 DEFAULT_INDEX = maxv<uint32>();
VulkanInstance *instance = nullptr;
LogicalDevice *device = nullptr;
AssetFetcher *loader = nullptr;
AssetSaver *saver = nullptr;
Shader *vrtxShader = nullptr, *fragShader = nullptr;
Renderpass *renderpass = nullptr;
GraphicsPipeline *pipeline = nullptr;
DescriptorPool *descPool;

vector<Texture2D*> textures;
vector<Image*> outputs;
vector<ImageView*> attachments;
vector<Framebuffer*> framebuffers;
vector<ConverterUniformBlock*> uniformBlocks;
std::atomic_uint64_t imageSaveCnt = 0;

/* Gets the wide output folder from the textures specific to a model. */
wstring GetOutputDirectory(const CLArgs &args)
{
	return args.Output.fileDirectory().toWide() + args.DisplayName.toWide() + L"\\";
}

/* Copies all the textures that don't need conversion to the output folder and returns the amount of textures that need to be converted. */
size_t DiskCopyTextures(PumIntermediate &data, const wstring &wdir, const ustring ldir)
{
	size_t result = 0;

	for (pum_texture &texture : data.Textures)
	{
		/* Only copy is the texture isn't marked for conversion. */
		result += !texture.ConversionCount;
		if (texture.ConversionCount)
		{
			const wstring srcPath = texture.Identifier.toWide();
			const wstring srcName = srcPath.fileName();
			const wstring dstPath = wdir + srcName;
			const ustring dstName = ldir + texture.Identifier.fileName();

			/* Let the OS copy the file over, we just set a new name in the intermediate. */
			if (FileReader::FileExists(srcPath))
			{
				FileWriter::CopyFile(srcPath, dstPath);
				texture.Identifier = dstName;
			}
			else Log::Warning("Texture file '%ls' could not be found!", srcName.c_str());
		}
	}

	/* Return the amount of textures that weren't converted. */
	return result;
}

bool InitializeVulkan(uint32 maxSets)
{
	/* Create the Vulkan instance. */
	instance = new VulkanInstance("Plutonium Content Compiler", false);
	const size_t physicalDeviceCnt = instance->GetPhysicalDevices().size();

	if (!physicalDeviceCnt)
	{
		Log::Error("Cannot compile textures without a Vulkan compatible physical device!");
		return false;
	}

	/* Create a logical device. */
	uint32 i = 0;
	for (const PhysicalDevice &physicalDevice : instance->GetPhysicalDevices())
	{
		/* Choose either the first discrete GPU or the last Vulkan device available. */
		if (physicalDevice.GetType() == PhysicalDeviceType::DiscreteGpu || i + 1 == physicalDeviceCnt)
		{
			/* Get the graphics and transfer queue families that we'll be using. */
			const uint32 graphicsQueueFamily = physicalDevice.GetBestGraphicsQueueFamily();
			const uint32 transferQueueFamily = physicalDevice.GetBestTransferQueueFamily();
			const uint32 same = graphicsQueueFamily == transferQueueFamily;

			/* Create the logical device. */
			const float priorities[] = { 1.0f, 1.0f };
			const DeviceQueueCreateInfo queueCreateInfos[] =
			{
				DeviceQueueCreateInfo(graphicsQueueFamily, 1 + same, priorities),
				DeviceQueueCreateInfo(transferQueueFamily, 1 + same, priorities)
			};

			const DeviceCreateInfo deviceCreateInfo{ 2 - same, queueCreateInfos };
			device = physicalDevice.CreateLogicalDevice(deviceCreateInfo);
			device->SetQueues(graphicsQueueFamily, 0, transferQueueFamily);
			break;
		}

		++i;
	}

	/* Create the asset loader for the image load process. */
	loader = new AssetFetcher(*device);
	saver = new AssetSaver(*device);
	saver->OnAssetSaved += [](const AssetSaver&, const Image&) { imageSaveCnt++; };

	/* Create the shaders needed for the conversion. */
	vrtxShader = new Shader(*device, VERTEX_SHADER, sizeof(VERTEX_SHADER), ShaderStageFlags::Vertex);
	fragShader = new Shader(*device, FRAGMENT_SHADER, sizeof(FRAGMENT_SHADER), ShaderStageFlags::Fragment);

	/* Create the subpass and renderpass. */
	Subpass subpass{ *device, { vrtxShader, fragShader } };
	subpass.SetDependency(PipelineStageFlags::FragmentShader, PipelineStageFlags::ColorAttachmentOutput, AccessFlags::None, AccessFlags::ColorAttachmentWrite, DependencyFlags::ByRegion);

	/* Add an external dependency so we can skip a memory barrier later on. */
	renderpass = new Renderpass(*device, std::move(subpass));
	renderpass->AddDependency(PipelineStageFlags::ColorAttachmentOutput, PipelineStageFlags::Transfer, AccessFlags::ColorAttachmentWrite, AccessFlags::TransferRead, DependencyFlags::ByRegion);

	/* Both the diffuse and specular/glossiness will transform from color attachments to transfer destinations after the subpass. */
	renderpass->PreCreate += [](Renderpass &renderpass)
	{
		Subpass &subpass = renderpass.GetSubpass(0);

		Output &diffuse = subpass.GetOutput("Diffuse");
		diffuse.SetFormat(Format::R8G8B8A8_UNORM);
		diffuse.SetLoadOperation(AttachmentLoadOp::DontCare);
		diffuse.SetInitialLayout(ImageLayout::Undefined);
		diffuse.SetLayout(ImageLayout::ColorAttachmentOptimal);
		diffuse.SetFinalLayout(ImageLayout::TransferSrcOptimal);

		Output &specGloss = subpass.GetOutput("SpecularGloss");
		specGloss.SetFormat(Format::R8G8B8A8_UNORM);
		specGloss.SetLoadOperation(AttachmentLoadOp::DontCare);
		specGloss.SetInitialLayout(ImageLayout::Undefined);
		specGloss.SetLayout(ImageLayout::ColorAttachmentOptimal);
		specGloss.SetFinalLayout(ImageLayout::TransferSrcOptimal);
	};

	renderpass->Initialize();

	/* Create the basic graphics pipeline. */
	pipeline = new GraphicsPipeline(*renderpass, 0);
	pipeline->SetCullMode(CullModeFlags::Front);
	pipeline->SetTopology(PrimitiveTopology::TriangleList);
	pipeline->AddDynamicState(DynamicState::ViewPort);
	pipeline->AddDynamicState(DynamicState::Scissor);
	pipeline->Finalize();

	/* Allocate a descriptor pool for all the material attributes. */
	descPool = new DescriptorPool(*renderpass, maxSets, 0, 0);
	return true;
}

/* Deletes all of the vulkan objects except for the instance. */
void FinalizeVulkan()
{
	delete pipeline;
	delete descPool;
	delete renderpass;

	delete vrtxShader;
	delete fragShader;

	delete saver;
	delete loader;
	delete device;
}

/* Get a unique set that defines all the pairs of source images. */
vector<std::pair<uint32, uint32>> GetSourcesSet(PumIntermediate &data)
{
	/* Get a list of the conversion sources, and remove the duplicated ones. */
	vector<std::pair<uint32, uint32>> result;
	for (pum_material &material : data.Materials)
	{
		if (material.IsFinalized) continue;

		/* Create a pair out of the albedo and metal/roughness texture. */
		std::pair<uint32, uint32> pair
		{
			material.HasDiffuseTexture ? material.DiffuseTexture : DEFAULT_INDEX,
			material.HasSpecularGlossTexture ? material.SpecGlossTexture : DEFAULT_INDEX
		};

		/* Check for duplicates. */
		if (!result.contains(pair)) result.emplace_back(pair);

		/* We need to add a new texture if the diffuse texture was not specified (this is pretty rare). */
		if (!material.HasDiffuseTexture)
		{
			pum_texture diffuseTexture;
			diffuseTexture.UsesLinearMagnification = true;
			diffuseTexture.UsesLinearMinification = true;
			diffuseTexture.UsesLinearMipmapMode = true;
			diffuseTexture.IsSRGB = true;
			diffuseTexture.ConversionCount = 0;

			data.Textures.emplace_back(diffuseTexture);
			material.SetDiffuseTexture(static_cast<uint32>(data.Textures.size()));
		}

		/* We also need to add a new texture for the specular gloss texture, this is more common as objects might be fullt metallic and fully rough. */
		if (!material.HasSpecularGlossTexture)
		{
			pum_texture specGlossTexture;
			specGlossTexture.UsesLinearMagnification = true;
			specGlossTexture.UsesLinearMinification = true;
			specGlossTexture.UsesLinearMipmapMode = true;
			specGlossTexture.ConversionCount = 0;

			data.Textures.emplace_back(specGlossTexture);
			material.SetSpecGlossTexture(static_cast<uint32>(data.Textures.size()));
		}
	}

	return result;
}

/* Loads all the individual textures in the source images. */
void LoadTextures(const PumIntermediate &data, const vector<std::pair<uint32, uint32>> &sources)
{
	/* Get a new set with just the texture indices. */
	vector<uint32> toLoad;
	toLoad.resize(data.Textures.size(), DEFAULT_INDEX);
	for (const auto[albedo, metalRough] : sources)
	{
		if (albedo != DEFAULT_INDEX) toLoad[albedo] = albedo;
		if (metalRough != DEFAULT_INDEX) toLoad[metalRough] = metalRough;
	}

	/* Load the model's textures. */
	textures.resize(toLoad.size(), nullptr);
	for (uint32 idx : toLoad)
	{
		if (idx != DEFAULT_INDEX)
		{
			/* The loader doesn't accept this intermediate format so we convert manually. */
			wstring path = data.Textures[idx].Identifier.toWide();
			textures[idx] = &loader->FetchTexture2D(path, SamplerCreateInfo(), false);
		}
	}

	/* Add the default texture (for albedo and metal/roughness) last. */
	textures.emplace_back(&loader->CreateTexture2D("Default", Color::White()));
}

void ReleaseResources()
{
	/* Release the source textures through the loader. */
	for (Texture2D *texture : textures)
	{
		if (texture) loader->Release(*texture);
	}

	/* Unload the uniform block, then the framebuffers, then the views, then the images. */
	for (const ConverterUniformBlock *block : uniformBlocks) delete block;
	for (const Framebuffer *framebuffer : framebuffers) delete framebuffer;
	for (const ImageView *view : attachments) delete view;
	for (const Image *image : outputs) delete image;
}

/* Creates the color attachments and framebuffers for the GPU conversion. */
void InitializeFramebuffers(const vector<std::pair<uint32, uint32>> &sources)
{
	/* We need to create framebuffers for every material and every material needs 2 textures. */
	framebuffers.reserve(sources.size());
	outputs.reserve(sources.size() << 1);
	attachments.reserve(outputs.capacity());

	for (auto &[albedo, metalRough] : sources)
	{
		/* The 2 output images have the same format and size. */
		const Extent3D size = textures[albedo != DEFAULT_INDEX ? albedo : metalRough]->GetImage().GetExtent();
		const ImageCreateInfo createInfo{ ImageType::Image2D, Format::R8G8B8A8_UNORM, size, 1, 1, SampleCountFlags::Pixel1Bit, ImageUsageFlags::ColorAttachment | ImageUsageFlags::TransferSrc };

		outputs.emplace_back(new Image(*device, createInfo));
		attachments.emplace_back(new ImageView(*outputs.back(), ImageViewType::Image2D, ImageAspectFlags::Color));
		outputs.emplace_back(new Image(*device, createInfo));
		attachments.emplace_back(new ImageView(*outputs.back(), ImageViewType::Image2D, ImageAspectFlags::Color));

		/* The framebuffer just uses the last 2 images. */
		framebuffers.emplace_back(new Framebuffer(*renderpass, size.To2D(), { attachments[attachments.size() - 2], attachments.back() }));
	}
}

/* Gets either the textures defined in the material or the default texture. */
Texture2D& GetTexture(bool has, uint32 idx)
{
	/* It is possible for the material to be marked to have a texture whilst that texture is currently a default texture. */
	if (has && textures[idx]) return *textures[idx];
	else return *textures.back();
}

/* Initializes all the material uniform blocks with their properties and textures. */
void InitializeUniformBlocks(const PumIntermediate &data)
{
	const DescriptorSetLayout &layout = renderpass->GetSubpass(0).GetSetLayout(0);

	for (const pum_material material : data.Materials)
	{
		if (material.IsFinalized) continue;

		ConverterUniformBlock *block = new ConverterUniformBlock(*descPool, layout, material.Metalness, 1.0f - material.Glossiness, material.DiffuseFactor);
		block->SetAlbedo(GetTexture(material.HasDiffuseTexture, material.DiffuseTexture));
		block->SetMetalRough(GetTexture(material.HasSpecularGlossTexture, material.SpecGlossTexture));

		uniformBlocks.emplace_back(block);
	}
}

/* Wait for all the source textures to be done loading. */
void WaitForLoads()
{
	bool loading;
	do
	{
		loading = false;

		for (const Texture2D *texture : textures)
		{
			if (texture && !texture->IsUsable())
			{
				loading = true;
				TaskScheduler::Help();
			}
		}
	} while (loading);
}

void WaitForSaves()
{
	while (imageSaveCnt.load() < outputs.size())
	{
		TaskScheduler::Help();
	}
}

wstring CreateOutputPath(const wstring &dir, PumIntermediate &data, uint32 srcIdx, uint32 otherIdx)
{
	wstring result = dir;

	/* We can just use the old name, if the source texture wasn't a default. */
	if (srcIdx != DEFAULT_INDEX)
	{
		pum_texture &src = data.Textures[srcIdx];
		result += src.Identifier.fileNameWithoutExtension().toWide();

		/* We need to add a suffix if the texture was already saved to disk before. */
		if (src.ConversionCount++)
		{
			result += L'_';
			result += wstring::from(src.ConversionCount);
		}

		/* Set the new identifier for the image. */
		src.Identifier = (result + L".png").toUTF32();
	}
	else
	{
		/* This used to be a default texture, so we reconstruct a name from the old texture. */
		pum_texture &other = data.Textures[otherIdx];
		result += other.Identifier.fileNameWithoutExtension().toWide();
		result += L"_Generated";

		/* Again we need to add a suffix if needed. */
		if (other.ConversionCount++)
		{
			result += L"_";
			result += wstring::from(other.ConversionCount);
		}

		/* We can always expect the first empty identifier to be our texture, this is because they are loaded and stored in the same order. */
		for (pum_texture &texture : data.Textures)
		{
			if (texture.Identifier.empty())
			{
				texture.Identifier = (result + L".png").toUTF32();
				break;
			}
		}
	}

	return result;
}

void SaveOutputs(PumIntermediate &data, const vector<std::pair<uint32, uint32>> &sources, const wstring &wdir)
{
	/* We don't know the name of outputs images that had a default texture as input. */
	size_t i = 0;
	for (auto[albedo, metalRough] : sources)
	{
		Image &diffuseImg = *outputs[i++];
		Image &specGlossImg = *outputs[i++];

		diffuseImg.OverrideState(ImageLayout::TransferSrcOptimal, AccessFlags::TransferRead);
		specGlossImg.OverrideState(ImageLayout::TransferSrcOptimal, AccessFlags::TransferRead);

		const wstring diffusePath = CreateOutputPath(wdir, data, albedo, metalRough);
		const wstring specGlossPath = CreateOutputPath(wdir, data, metalRough, albedo);

		saver->SaveImage(diffuseImg, diffusePath, ImageSaveFormats::Png);
		saver->SaveImage(specGlossImg, specGlossPath, ImageSaveFormats::Png);
	}
}

void ConvertTextures(PumIntermediate & data, const wstring &wdir, const ustring ldir)
{
	/* Load all of the textures. */
	vector<std::pair<uint32, uint32>> sources = GetSourcesSet(data);
	LoadTextures(data, sources);

	/* Create all of the resources that aren't dependent on the textures being loaded. */
	InitializeFramebuffers(sources);
	InitializeUniformBlocks(data);

	/* Get the queue we'll be pushing towards and create some command buffers to hopefully speed up converting. */
	Queue &queue = device->GetGraphicsQueue(0);
	CommandPool pool{ *device, device->GetGraphicsQueueFamily(), CommandPoolCreateFlags::Transient };
	vector<CommandBuffer> cmdBuffers;
	cmdBuffers.reserve(framebuffers.size() + 1);
	for (size_t i = 0; i < cmdBuffers.capacity(); i++) cmdBuffers.emplace_back(std::move(pool.Allocate()));

	/* We can create the memory barries before the textures are done loading, we just can't record them yet. */
	vector<std::pair<const Image*, ImageSubresourceRange>> srcMemoryBarriers;
	srcMemoryBarriers.reserve(textures.size());
	for (const Texture2D *texture : textures)
	{
		if (texture) srcMemoryBarriers.emplace_back(&texture->GetImage(), texture->GetFullRange());
	}

	/* We have to wait for the source textures to be done loading before we can make any render calls. */
	WaitForLoads();

	/* Make sure all the resources are in the correct positions. */
	CommandBuffer &memoryCmdBuffer = cmdBuffers.front();
	memoryCmdBuffer.Begin();
	memoryCmdBuffer.MemoryBarrier(srcMemoryBarriers, PipelineStageFlags::Transfer, PipelineStageFlags::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlags::ShaderRead);
	descPool->Update(memoryCmdBuffer, PipelineStageFlags::FragmentShader);
	memoryCmdBuffer.End();
	queue.Submit(memoryCmdBuffer);

	/* Render all the source textures to the framebuffers in one go. */
	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		CommandBuffer &cmdBuffer = cmdBuffers[i + 1];
		cmdBuffer.Begin();

		cmdBuffer.BeginRenderPass(*renderpass, *framebuffers[i], SubpassContents::Inline);
		cmdBuffer.BindGraphicsPipeline(*pipeline);
		cmdBuffer.BindGraphicsDescriptor(*pipeline, *uniformBlocks[i]);
		cmdBuffer.SetViewportAndScissor(framebuffers[i]->GetArea());
		cmdBuffer.Draw(3, 1, 0, 0);
		cmdBuffer.EndRenderPass();

		cmdBuffer.End();
		queue.Submit(cmdBuffer);
	}

	/* Wait for the graphics queue to be done executing our commands. */
	device->GetGraphicsQueue(0).WaitIdle();

	/* We must wait for the resources to be finalized before we can release them. */
	SaveOutputs(data, sources, wdir);
	WaitForSaves();

	ReleaseResources();
}

void CopyAndConvertMaterials(PumIntermediate & data, const CLArgs & args)
{
	/* Get the output directory as a wide and long string (we for paths and long for identifiers). */
	const wstring wdir = GetOutputDirectory(args);
	const ustring ldir = wdir.toUTF32();

	/* Precreate the directory to avoid race conditions. */
	Profiler::Begin("Copying textures");
	if (data.Textures.size()) FileWriter::CreateDirectory(wdir);

	/* Copy over all the finalized textures and log the amount of textures that will be parsed. */
	const size_t parseCnt = DiskCopyTextures(data, wdir, ldir);
	Profiler::End();
	if (!parseCnt) return;
	Log::Message("Converting %zu textures from metal/roughness to specular/glossiness.", parseCnt);

	/* The amount of descriptor sets if the same as the material count. */
	uint32 setCnt = 0;
	for (pum_material &mat : data.Materials) setCnt += !mat.IsFinalized;

	/* Make sure that we only start the conversion if all the component were properly created. */
	Profiler::Begin("Converting texture");
	if (InitializeVulkan(setCnt))
	{
		ConvertTextures(data, wdir, ldir);
		FinalizeVulkan();
	}

	/* The instance is always created so always delete that. */
	delete instance;
	Profiler::End();
}