#pragma once
#include "Core/EnumUtils.h"
#include "Core/Diagnostics/Logging.h"

#ifdef _DEBUG
#define VK_VALIDATE(result, proc)	Pu::ValidateVkApiResult(result, #proc)
#else
#define VK_VALIDATE(result, proc)	result
#endif

namespace Pu
{
	/* Possible values of the second group of four bytes in the header returnd by getPipelineCacheData. */
	enum class PipelineCacheHeaderVersion
	{
		/* Version 1. */
		One = 1
	};

	/* Defines the result codes returned by Vulkan. */
	enum class VkApiResult
	{
		/* Command successfully completed. */
		Success = 0,
		/* A fence or query has not yet completed. */
		NotReady = 1,
		/* A wait operation has not completed in the specified time. */
		Timeout = 2,
		/* An event is signaled. */
		EventSet = 3,
		/* An event is unsignaled. */
		EventReset = 4,
		/* A return array was too small for the result. */
		Incomplete = 5,
		/* A swapchain no longer matches the surface properties exactly. */
		SuboptimalKhr = 1000001003,

		/* A host memory allocation has failed. */
		HostOutOfMemory = -1,
		/* A device memory allocation has failed. */
		OutOfDeviceMemory = -2,
		/* Initialization of an object could not be completed. */
		InitializationFailed = -3,
		/* The logical or physical device has been lost. */
		DeviceLost = -4,
		/* Mapping of a memory object has failed. */
		MemoryMapFailed = -5,
		/* A requested layer is not present or could not be loaded. */
		LayerNotPresent = -6,
		/* A requested extension is not supported. */
		ExtensionNotPresent = -7,
		/* A requested feature is not supported. */
		FeatureNotPresent = -8,
		/* The requested version of Vulkan is not supported. */
		IncompatibleDriver = -9,
		/* To many objects of the type have been created. */
		TooManyObjects = -10,
		/* A requested format is not supported on this device. */
		FormatNotSupported = -11,
		/* A pool allocation has failed due to fragmentation of the pool's memory. */
		FragmentedPool = -12,
		/* A pool allocation has failed due to memory shortage. */
		OutOfPoolMemory = -1000069000,
		/* The handle passed to the command was invalid. */
		InvalidExternalHandle = -1000072003,
		/* A surface is no longer available. */
		SurfaceLostKhr = -1000000000,
		/* The native window is already in use by Vulkan or another API. */
		NativeWindowInUseKhr = -1000000001,
		/* A surface has changed in such a way that it's no longer compatible with the swapchain. */
		OutOfDateKhr = -1000001004,
		/* The display used by the swapchain does not use the same presentable image layout. */
		IncompatibleDisplayKhr = -1000003001,
		/* The validation function has failed. */
		ValidationFailedExt = -1000011001,
		/* One or more shaders failed to compile or link. */
		InvalidShaderNv = -1000012000,
		/* A descriptor pool creation has failed due to fragmentation. */
		FragmentationExt = -1000161000,
		/* The command was not permitted. */
		NotPermittedExt = -1000174001
	};

	/* Defines the types of structures supported by the Vulkan API. */
	enum class StructureType
	{
		ApplicationInfo = 0,
		InstanceCreateInfo = 1,
		DeviceQueueCreateInfo = 2,
		DeviceCreatInfo = 3,
		SubmitInfo = 4,
		MemoryAllocateInfo = 5,
		MappedMemoryRange = 6,
		BindSpareInfo = 7,
		FenceCreateInfo = 8,
		SemaphoreCreateInfo = 9,
		EventCreateInfo = 10,
		QueryPoolCreatInfo = 11,
		BufferCreateInfo = 12,
		BufferViewCreateInfo = 13,
		ImageCreateInfo = 14,
		ImageViewCreateInfo = 15,
		ShaderModuleCreateInfo = 16,
		PipelineCacheCreateInfo = 17,
		PipelineShaderStageCreateInfo = 18,
		PipelineVertexInputStateCreateInfo = 19,
		PipelineInputAssemblyStateCreateInfo = 20,
		PipelineTessellationStateCreateInfo = 21,
		PipelineViewportStateCreateInfo = 22,
		PipelineRasterizationStateCreateInfo = 23,
		PipelineMultiSampleStateCreateInfo = 24,
		PipelineDepthStencilStateCreateInfo = 25,
		PipelineColorBlendStateCreateInfo = 26,
		PipelineDynamicStateCreateInfo = 27,
		GraphicsPipelineCreateInfo = 28,
		ComputePipelineCreateInfo = 29,
		PipelineLayourCreateInfo = 30,
		SamplerCreateInfo = 31,
		DescriptorSetLayourCreateInfo = 32,
		DescriptorPoolCreateInfo = 33,
		DescriptorSetAllocateInfo = 34,
		WriteDescriptorSet = 35,
		CopyDescriptorSet = 36,
		FramebufferCreateInfo = 37,
		RenderPassCreateInfo = 38,
		CommandPoolCreateInfo = 39,
		CommandBufferAllocateInfo = 40,
		CommandBufferInheritanceInfo = 41,
		CommandBufferBeginInfo = 42,
		RenderPassBeginInfo = 43,
		BufferMemoryBarrier = 44,
		ImageMemoryBarrier = 45,
		MemoryBarrier = 46,
		LoaderInstanceCreateInfo = 47,
		LoaderDeviceCreateInfo = 48,
		SwapChainCreateInfoKhr = 1000001000,
		PresentInfoKhr = 1000001001,
		DisplayModeCreateInfoKhr = 1000002000,
		DisplaySurfaceCreateInfo = 1000002001,
		DisplayPresentInfoKhr = 1000003000,
		XlibSurfaceCreateInfoKhr = 1000004000,
		XCBSurfaceCreateInfoKhr = 1000005000,
		WaylandSurfaceCreateInfoKhr = 1000006000,
		MirSurfaceCreateInfoKhr = 1000007000,
		AndroidSurfaceCreateInfoKhr = 1000008000,
		Win32SurfaceCreateInfoKhr = 1000009000,
		DebugReportCreateInfoExt = 1000011000,
		DebugUtilsLabelExt = 1000128002,
		DebugUtilsObjectNameInfoExt = 1000128000,
		DebugUtilsMessangerCallbackDataExt = 1000128003,
		DebugUtilsMessengerCreateInfoExt = 1000128004,
	};

	/* Defines the lifetime of a system allocation. */
	enum class SystemAllocationScope
	{
		/* Allocation is scoped to the duration of the Vulkan command. */
		Command = 0,
		/* Allocation is scoped to the Vulkan object that is begin created or used. */
		Object = 1,
		/* Allocation is scoped to the lifetime of a PipelineCache or ValidationCache object. */
		Cache = 2,
		/* Allocation is scoped to the lifetime of the Vulkan device. */
		Device = 3,
		/* Allocation is scoped to the lifetime of the Vulkan instance. */
		Instance = 4
	};

	/* Defines the type of internal allocation. */
	enum class InternalAllocationType
	{
		/* The allocation is intended for excecution by the host. */
		Executable = 0
	};

	/* Defines the formats for Vulkan images. */
	enum class Format
	{
		Undefined = 0,
		R4G4_UNORM_PACK8 = 1,
		R4G4B4A4_UNORM_PACK16 = 2,
		B4G4R4A4_UNORM_PACK16 = 3,
		R5G6B5_UNORM_PACK16 = 4,
		B5G6R5_UNORM_PACK16 = 5,
		R5G5B5A1_UNORM_PACK16 = 6,
		B5G5R5A1_UNORM_PACK16 = 7,
		A1R5G5B5_UNORM_PACK16 = 8,
		R8_UNORM = 9,
		R8_SNORM = 10,
		R8_USCALED = 11,
		R8_SSCALED = 12,
		R8_UINT = 13,
		R8_SINT = 14,
		R8_SRGB = 15,
		R8G8_UNORM = 16,
		R8G8_SNORM = 17,
		R8G8_USCALED = 18,
		R8G8_SSCALED = 19,
		R8G8_UINT = 20,
		R8G8_SINT = 21,
		R8G8_SRGB = 22,
		R8G8B8_UNORM = 23,
		R8G8B8_SNORM = 24,
		R8G8B8_USCALED = 25,
		R8G8B8_SSCALED = 26,
		R8G8B8_UINT = 27,
		R8G8B8_SINT = 28,
		R8G8B8_SRGB = 29,
		B8G8R8_UNORM = 30,
		B8G8R8_SNORM = 31,
		B8G8R8_USCALED = 32,
		B8G8R8_SSCALED = 33,
		B8G8R8_UINT = 34,
		B8G8R8_SINT = 35,
		B8G8R8_SRGB = 36,
		R8G8B8A8_UNORM = 37,
		R8G8B8A8_SNORM = 38,
		R8G8B8A8_USCALED = 39,
		R8G8B8A8_SSCALED = 40,
		R8G8B8A8_UINT = 41,
		R8G8B8A8_SINT = 42,
		R8G8B8A8_SRGB = 43,
		B8G8R8A8_UNORM = 44,
		B8G8R8A8_SNORM = 45,
		B8G8R8A8_USCALED = 46,
		B8G8R8A8_SSCALED = 47,
		B8G8R8A8_UINT = 48,
		B8G8R8A8_SINT = 49,
		B8G8R8A8_SRGB = 50,
		A8B8G8R8_UNORM_PACK32 = 51,
		A8B8G8R8_SNORM_PACK32 = 52,
		A8B8G8R8_USCALED_PACK32 = 53,
		A8B8G8R8_SSCALED_PACK32 = 54,
		A8B8G8R8_UINT_PACK32 = 55,
		A8B8G8R8_SINT_PACK32 = 56,
		A8B8G8R8_SRGB_PACK32 = 57,
		A2R10G10B10_UNORM_PACK32 = 58,
		A2R10G10B10_SNORM_PACK32 = 59,
		A2R10G10B10_USCALED_PACK32 = 60,
		A2R10G10B10_SSCALED_PACK32 = 61,
		A2R10G10B10_UINT_PACK32 = 62,
		A2R10G10B10_SINT_PACK32 = 63,
		A2B10G10R10_UNORM_PACK32 = 64,
		A2B10G10R10_SNORM_PACK32 = 65,
		A2B10G10R10_USCALED_PACK32 = 66,
		A2B10G10R10_SSCALED_PACK32 = 67,
		A2B10G10R10_UINT_PACK32 = 68,
		A2B10G10R10_SINT_PACK32 = 69,
		R16_UNORM = 70,
		R16_SNORM = 71,
		R16_USCALED = 72,
		R16_SSCALED = 73,
		R16_UINT = 74,
		R16_SINT = 75,
		R16_SFLOAT = 76,
		R16G16_UNORM = 77,
		R16G16_SNORM = 78,
		R16G16_USCALED = 79,
		R16G16_SSCALED = 80,
		R16G16_UINT = 81,
		R16G16_SINT = 82,
		R16G16_SFLOAT = 83,
		R16G16B16_UNORM = 84,
		R16G16B16_SNORM = 85,
		R16G16B16_USCALED = 86,
		R16G16B16_SSCALED = 87,
		R16G16B16_UINT = 88,
		R16G16B16_SINT = 89,
		R16G16B16_SFLOAT = 90,
		R16G16B16A16_UNORM = 91,
		R16G16B16A16_SNORM = 92,
		R16G16B16A16_USCALED = 93,
		R16G16B16A16_SSCALED = 94,
		R16G16B16A16_UINT = 95,
		R16G16B16A16_SINT = 96,
		R16G16B16A16_SFLOAT = 97,
		R32_UINT = 98,
		R32_SINT = 99,
		R32_SFLOAT = 100,
		R32G32_UINT = 101,
		R32G32_SINT = 102,
		R32G32_SFLOAT = 103,
		R32G32B32_UINT = 104,
		R32G32B32_SINT = 105,
		R32G32B32_SFLOAT = 106,
		R32G32B32A32_UINT = 107,
		R32G32B32A32_SINT = 108,
		R32G32B32A32_SFLOAT = 109,
		R64_UINT = 110,
		R64_SINT = 111,
		R64_SFLOAT = 112,
		R64G64_UINT = 113,
		R64G64_SINT = 114,
		R64G64_SFLOAT = 115,
		R64G64B64_UINT = 116,
		R64G64B64_SINT = 117,
		R64G64B64_SFLOAT = 118,
		R64G64B64A64_UINT = 119,
		R64G64B64A64_SINT = 120,
		R64G64B64A64_SFLOAT = 121,
		B10G11R11_UFLOAT_PACK32 = 122,
		E5B9G9R9_UFLOAT_PACK32 = 123,
		D16_UNORM = 124,
		X8_D24_UNORM_PACK32 = 125,
		D32_SFLOAT = 126,
		S8_UINT = 127,
		D16_UNORM_S8_UINT = 128,
		D24_UNORM_S8_UINT = 129,
		D32_SFLOAT_S8_UINT = 130,
		BC1_RGB_UNORM_BLOCK = 131,
		BC1_RGB_SRGB_BLOCK = 132,
		BC1_RGBA_UNORM_BLOCK = 133,
		BC1_RGBA_SRGB_BLOCK = 134,
		BC2_UNORM_BLOCK = 135,
		BC2_SRGB_BLOCK = 136,
		BC3_UNORM_BLOCK = 137,
		BC3_SRGB_BLOCK = 138,
		BC4_UNORM_BLOCK = 139,
		BC4_SNORM_BLOCK = 140,
		BC5_UNORM_BLOCK = 141,
		BC5_SNORM_BLOCK = 142,
		BC6H_UFLOAT_BLOCK = 143,
		BC6H_SFLOAT_BLOCK = 144,
		BC7_UNORM_BLOCK = 145,
		BC7_SRGB_BLOCK = 146,
		ETC2_R8G8B8_UNORM_BLOCK = 147,
		ETC2_R8G8B8_SRGB_BLOCK = 148,
		ETC2_R8G8B8A1_UNORM_BLOCK = 149,
		ETC2_R8G8B8A1_SRGB_BLOCK = 150,
		ETC2_R8G8B8A8_UNORM_BLOCK = 151,
		ETC2_R8G8B8A8_SRGB_BLOCK = 152,
		EAC_R11_UNORM_BLOCK = 153,
		EAC_R11_SNORM_BLOCK = 154,
		EAC_R11G11_UNORM_BLOCK = 155,
		EAC_R11G11_SNORM_BLOCK = 156,
		ASTC_4x4_UNORM_BLOCK = 157,
		ASTC_4x4_SRGB_BLOCK = 158,
		ASTC_5x4_UNORM_BLOCK = 159,
		ASTC_5x4_SRGB_BLOCK = 160,
		ASTC_5x5_UNORM_BLOCK = 161,
		ASTC_5x5_SRGB_BLOCK = 162,
		ASTC_6x5_UNORM_BLOCK = 163,
		ASTC_6x5_SRGB_BLOCK = 164,
		ASTC_6x6_UNORM_BLOCK = 165,
		ASTC_6x6_SRGB_BLOCK = 166,
		ASTC_8x5_UNORM_BLOCK = 167,
		ASTC_8x5_SRGB_BLOCK = 168,
		ASTC_8x6_UNORM_BLOCK = 169,
		ASTC_8x6_SRGB_BLOCK = 170,
		ASTC_8x8_UNORM_BLOCK = 171,
		ASTC_8x8_SRGB_BLOCK = 172,
		ASTC_10x5_UNORM_BLOCK = 173,
		ASTC_10x5_SRGB_BLOCK = 174,
		ASTC_10x6_UNORM_BLOCK = 175,
		ASTC_10x6_SRGB_BLOCK = 176,
		ASTC_10x8_UNORM_BLOCK = 177,
		ASTC_10x8_SRGB_BLOCK = 178,
		ASTC_10x10_UNORM_BLOCK = 179,
		ASTC_10x10_SRGB_BLOCK = 180,
		ASTC_12x10_UNORM_BLOCK = 181,
		ASTC_12x10_SRGB_BLOCK = 182,
		ASTC_12x12_UNORM_BLOCK = 183,
		ASTC_12x12_SRGB_BLOCK = 184,
	};

	/* Defines the basic dimensionality of an image. */
	enum class ImageType
	{
		/* Specifies a one-dimensional image. */
		Image1D = 0,
		/* Specified a two-dimensional image. */
		Image2D = 1,
		/* Specified a three-dimensional image. */
		Image3D = 2
	};

	/* Defines the tiling arrangement of data elements. */
	enum class ImageTiling
	{
		/* Texels are laid out in an implementation-dependent arrangement. */
		Optimal = 0,
		/* Textels are laid out in memory in row-major order. */
		Linear = 1
	};

	/* Defines the types of phsyical devices. */
	enum class PhysicalDeviceType
	{
		/* The device does not match any other type. */
		Other = 0,
		/* Embedded in or tightly coupled with the host. */
		IntegratedGpu = 1,
		/* Separate processor connected to the host via an interlink. */
		DiscreteGpu = 2,
		/* A virtual node in a virtualization enviroment. */
		VirtualGpu = 3,
		/* Same processors as the host. */
		CPU = 4,
	};

	/* Defines the type of queries managed by a pool. */
	enum class QueryType
	{
		/* Specifies an occlusion query. */
		Occlusion = 0,
		/* Specifies a pipeline statistics query. */
		PipelineStatistics = 1,
		/* Specifies a timestamp query. */
		Timestamp = 2,
	};

	/* Defines the available queue sharing modes for buffer and image object. */
	enum class SharingMode
	{
		/* Any range or image subresource of the obejct is exclusive to some queue family. */
		Exclusive = 0,
		/* Any range or image subresource of the object can be accessed via multiple queue families. */
		Concurrent = 1
	};

	/* Defines the set of image layouts. */
	enum class ImageLayout
	{
		Undefined = 0,
		General = 1,
		ColorAttachmentOptimal = 2,
		DepthStencilAttachmentOptimal = 3,
		DepthStencilReadOnlyOptimal = 4,
		ShaderReadOnlyOptimal = 5,
		TransferSrcOptimal = 6,
		TransferDstOptimal = 7,
		Preinitialized = 8,
		DepthReadOnlyStencilAttachmentOptimal = 1000117000,
		DepthAttachmentStencilReadOnlyOptimal = 1000117001,
		PresentSrcKhr = 1000001002,
		SharedPresentKhr = 1000111000,
		ShadingRateOptimalNv = 1000164003
	};

	/* Defines the types of image views that can be created. */
	enum class ImageViewType
	{
		Image1D = 0,
		Image2D = 1,
		Image3D = 2,
		ImageCube = 3,
		Image1DArray = 4,
		Image2dArray = 5,
		ImageCubeArray = 6
	};

	/* Defines the component values placed in each component of the output vector. */
	enum class ComponentSwizzle
	{
		/* Components are set to identity swizzle. */
		Identity = 0,
		/* Components are set to zero. */
		Zero = 1,
		/* Components are either set to 1 or 1.0. */
		One = 2,
		/* Components are set to the value of the R component. */
		R = 3,
		/* Components are set to the value of the G component. */
		G = 4,
		/* Components are set to the value of the B component. */
		B = 5,
		/* Components are set to the value of the A component. */
		A = 6
	};

	/* Specified the rate at which vertex attributes are pulled from buffers. */
	enum class VertexInputRate
	{
		/* The vertex attribute addressing is a function of the vertex index. */
		Vertex = 0,
		/* The vertex attribute addressing is a function of the instance index. */
		Instance = 1
	};

	/* Defines how vertices can be organized into primitives. */
	enum class PrimitiveTopology
	{
		PointList = 0,
		LineList = 1,
		LineStrip = 2,
		TriangleList = 3,
		TriangleStrip = 4,
		TriangleFan = 5,
		LineListWithAdjacency = 6,
		LineStripWithAdjacency = 7,
		TriangleListWithAdjacency = 8,
		TriangleStripWithAdjacency = 9,
		PatchList = 10
	};

	/* Defines hwo polygons can be resterized. */
	enum class PolygonMode
	{
		/* Polygons are rendered as rasterized polygons. */
		Fill = 0,
		/* Polygons are rendered as lines. */
		Line = 1,
		/* Polygons are rendered as points. */
		Point = 2
	};

	/* Defines which side to consider the front face of a triangle. */
	enum class FrontFace
	{
		/* A triangle with the positive area is the front face. */
		CounterClockwise = 0,
		/* A traingle with the negative area is the front face. */
		Clockwise = 1
	};

	/* Defines the stencil comparison functions. */
	enum class CompareOp
	{
		/* The test never passes. */
		Never = 0,
		/* The test passes when R < S. */
		Less = 1,
		/* The test passes when R == S */
		Equal = 2,
		/* The test passes when R <= S. */
		LessOrEqual = 3,
		/* The test passes when R > S. */
		Greater = 4,
		/* The test passes when R != S. */
		NotEqual = 5,
		/* The test passes when R >= S. */
		GreaterorEqual = 6,
		/* The test always passes. */
		Always = 7
	};

	/* Defines what can happen to the stored stencil value if this or certain subsequent tests fail or pass. */
	enum class StencilOp
	{
		/* Keeps the current value. */
		Keep = 0,
		/* Set the value to zero. */
		Zero = 1,
		/* Replaces the value with refrence value. */
		Replace = 2,
		/* Increments the current value and clamps to the maximum representable unsigned value. */
		IncrementAndClamp = 3,
		/* Decrements the current value and clamps to zero. */
		DecrementAndClamp = 4,
		/* Bitwise inverts the current value. */
		Invert = 5,
		/* Increments the current value and wraps to zero when the maximum value would have been exceeded. */
		IncrementAndWrap = 6,
		/* Decrements the current value and wraps to the maximum value when the value would go below zero. */
		DecrementAndWrap = 7
	};

	/* Defines the logical operations during color blending. */
	enum class LogicOp
	{
		/* Sets the value to zero. */
		Clear = 0,
		/* Sets the value to S & D. */
		And = 1,
		/* Sets the value to S & ~D. */
		AndReverse = 2,
		/* Sets the value to S. */
		Copy = 3,
		/* Sets the value to ~S & D. */
		AndIverted = 4,
		/* Sets the value to D. */
		NoOp = 5,
		/* Sets the value to S ^ D. */
		Xor = 6,
		/* Sets the value to S | D. */
		Or = 7,
		/* Sets the value to ~(S | D). */
		Nor = 8,
		/* Sets the value to ~(S ^ D). */
		Equivalent = 9,
		/* Sets the value to ~D. */
		Invert = 10,
		/* Sets the value to S | ~D. */
		OrReverse = 11,
		/* Sets the value to ~S. */
		CopyInverted = 12,
		/* Sets the value to ~S | D. */
		OrInverted = 13,
		/* Sets the value to ~(S & D). */
		Nand = 14,
		/* Sets the value to one. */
		Set = 15
	};

	/*
	Defines the source and destination color and alpha blending factors.
	s0 is the first source color.
	s1 is the second source color.
	d is the destination color.
	c is a specified constant color.
	*/
	enum class BlendFactor
	{
		/* [0, 0, 0, 0]. */
		Zero = 0,
		/* [1, 1, 1, 1]. */
		One = 1,
		/* [Rs0, Gs0, Bs0, As0] */
		SrcColor = 2,
		/* [1-Rs0, 1-Gs0, 1-Bs0, 1-As0] */
		ISrcColor = 3,
		/* [Rd, Gd, Bd, Ad]. */
		DstColor = 4,
		/* [1-Rd, 1-Gd, 1-Bd, 1-Ad]. */
		IDstColor = 5,
		/* [As0, As0, As0, As0]. */
		SrcAlpha = 6,
		/* [1-As0, 1-As0, 1-As0, 1-As0]. */
		ISrcAlpha = 7,
		/* [Ad, Ad, Ad, Ad]. */
		DstAlpha = 8,
		/* [1-Ad, 1-Ad, 1-Ad, 1-Ad]. */
		IDstAlpa = 9,
		/* [Rc, Gc, Bc, Ac]. */
		ConstColor = 10,
		/* [1-Rc, 1-Gc, 1-Bc, 1-Ac]. */
		IConstColor = 11,
		/* [Ac, Ac, Ac, Ac]. */
		ConstAlpha = 12,
		/* [1-Ac, 1-Ac, 1-Ac, 1-Ac]. */
		IConstAlpha = 13,
		/* [f, f, f, 1] where f = min(As0, 1-Ad). */
		SrcAlphaSaturate = 14,
		/* [Rs1, Gs1, Bs1, As1] */
		Src1Color = 15,
		/* [1-Rs1, 1-Gs1, 1-Bs1, 1-As1] */
		ISrc1Color = 16,
		/* [As1, As1, As1, As1]. */
		Src1Alpha = 17,
		/* [1-As1, 1-As1, 1-As1, 1-As1]. */
		ISrc1Alpha = 18
	};

	/* Defines the types of color blending operations. */
	enum class BlendOp
	{
		/* Adds the components together. */
		Add = 0,
		/* Subtracts the destination from the source. */
		Subtract = 1,
		/* Subtracts the source from the destination. */
		ReverseSubtract = 2,
		/* Sets the components to the minimum value. */
		Min = 3,
		/* Sets the components to the maximum value. */
		Max = 4
	};

	/* Defines the locations avialble for dynamic changes in a pipelne. */
	enum class DynamicState
	{
		ViewPort = 0,
		Scissor = 1,
		LineWidth = 2,
		DepthBias = 3,
		BlendConstants = 4,
		DepthBounds = 5,
		StencilCompareMask = 6,
		StencilWriteMask = 7,
		StencilReference = 8,
	};

	/* Defines the filters used for texture lookups. */
	enum class Filter
	{
		/* Defines nearest filtering. */
		Nearest = 0,
		/* Defines linear filtering. */
		Linear = 1,
		/* Defines subic filtering. */
		CubicImg = 1000015000
	};

	/* Defines the mipmap mode used for texture lookups. */
	enum class SamplerMipmapMode
	{
		/* Defines nearest filtering. */
		Nearest = 0,
		/* Defines linear filtering. */
		Linear = 1
	};

	/* Defines the behaviour of sampling with coordinatees outide of the range [0, 1]. */
	enum class SamplerAddressMode
	{
		/* The coordinates will wrap back to the other side of the range until a valid value is found. */
		Repeat = 0,
		/* The coordinates will change direction until a valid value is found. */
		MirroredRepeat = 1,
		/* The coordinates will take the value of the closest valid fragment. */
		ClampToEdge = 2,
		/* The coordinates will take the value of the border value. */
		ClampToBorder = 3,
	};

	/* Defines the possible border colors for texture lookups. */
	enum class BorderColor
	{
		/* [0.0, 0.0, 0.0, 0.0]. */
		FloatTransparentBlack = 0,
		/* [0, 0, 0, 0]. */
		IntTransparentBlack = 1,
		/* [0.0, 0.0, 0.0, 1.0]. */
		FloatOpaqueBlack = 2,
		/* [0, 0, 0, 1]. */
		IntOpaqueBlack = 3,
		/* [1.0, 1.0, 1.0, 1.0]. */
		FloatOpaqueWhite = 4,
		/* [1, 1, 1, 1]. */
		IntOpaqueWhite = 5
	};

	/* Defines the types of descriptors. */
	enum class DescriptorType
	{
		Sampler = 0,
		CombinedImageSampler = 1,
		SampledImage = 2,
		StorageImage = 3,
		UniformTexelBuffer = 4,
		StorageTexelBuffer = 5,
		UniformBuffer = 6,
		StorageBuffer = 7,
		UniformBufferDynamic = 8,
		StorageBufferDynamic = 9,
		InputAttachment = 10
	};

	/* Defines how the contents of an attachment are treated. */
	enum class AttachmentLoadOp
	{
		/* The previous contents of the image within the render area are preserved. */
		Load = 0,
		/* The contents of the image within the render area are cleared to a uniform value. */
		Clear = 1,
		/* The contents within the area need not be preserved; the contents will be undefined outside this area. */
		DontCare = 2
	};

	/* Defines how the contents of an attachment are treated. */
	enum class AttachmentStoreOp
	{
		/* The contents during the render pass and within the render area are written to memory. */
		Store = 0,
		/* The contents within the render area are not needed after rendering. */
		DontCare = 1
	};

	/* Defines the bind points of a pipeline object. */
	enum class PipelineBindPoint
	{
		/* Binding as a graphics pipeline. */
		Graphics = 0,
		/* Binding as a compute pipeline. */
		Compute = 1
	};

	/* Defines the command buffer levels. */
	enum class CommandBufferLevel
	{
		/* Specifies a primary command buffer. */
		Primary = 0,
		/* Specifies a secondary command buffer. */
		Secondary = 1
	};

	/* Specifies the size of indices. */
	enum class IndexType
	{
		/* Indices are 16-bit unsigned integers. */
		UInt16 = 0,
		/* Indices are 32-bit unsigned integers. */
		UInt32 = 1
	};

	/* Specifies how the command in the first subpass will be provided. */
	enum class SubpassContents
	{
		/* The contents of the subpass will be recorded inline in the primary command buffer. */
		Inline = 0,
		/* The contents of the subpass will be recorded in secondary command buffers. */
		SecondaryCommandBuffers = 1
	};

	/* Defines the bits that can be set in the FormatProperties features. */
	enum class FormatFeatureFlag
	{
		/* Specifies that no features are enabled. */
		None = 0x00000000,
		/* Specifies that an image view can be sampled from. */
		SampledImage = 0x00000001,
		/* Specifies that an image view can be used as storage. */
		StorageImage = 0x00000002,
		/* Specifies that an image view can be used as storage that supports atomic operations. */
		StorageImageAtomic = 0x00000004,
		/* Specifies that an image view can be used to create a buffer view that can be bound to a uniform texel descriptor. */
		UniformTexelBuffer = 0x00000008,
		/* Specifies that an image view can be used to create a buffer view that can be bound to a storage texel descriptor. */
		StorageTexelBuffer = 0x00000010,
		/* Specifies that an image view can be used to create a buffer view that can be bound to a storage texel descriptor that supports atomic operations. */
		StorageTexelBufferAtomic = 0x00000020,
		/* Specifies that the format can be used as a vertex attribute format. */
		VertexBuffer = 0x00000040,
		/* Specifies that an image view can be used as a framebuffer color attachment. */
		ColorAttachment = 0x00000080,
		/* Specifies that an image view can be used as a framebuffer color attachment that supports blending. */
		ColorAttachmentBlend = 0x00000100,
		/* Specifies that an image view can be used as a framebuffer depth/stencil attachment. */
		DepthStencilAttachment = 0x00000200,
		/* Specifies that an image can be used as srcImage in the CmdBlitImage command. */
		BlirSrc = 0x00000400,
		/* Specifies that an image can be used as dstImage in the CmdBlitImage command. */
		BlitDst = 0x00000800,
		/* Specifies that an image can be used with a sampler that has either magFilter or minFilter set to Linear. */
		SampledImageFilterLinear = 0x00001000,
	};

	/* Defines the bits that can be set to specify the intended usage of an image. */
	enum class ImageUsageFlag
	{
		/* No flags are set. */
		None = 0x00000000,
		/* Specifies that the image can be used as the source of a transfer command. */
		TransferSrc = 0x00000001,
		/* Specified that the image can be used as the destination of a transfer command. */
		TransferDst = 0x00000002,
		/* Specifies that the image can be used to create an image view and can be sampled by a shader. */
		Sampled = 0x00000004,
		/* Specifies that the image can be used to create an image view. */
		Storage = 0x00000008,
		/* Specifies that the image can be used to create an image view suitable for use as a color or resolve attachment. */
		ColorAttachment = 0x00000010,
		/* Specifies that the image can be used to create an image view suitable for use as a depth/stencil attachment. */
		DepthStencilAttachment = 0x00000020,
		/* Specifies that the memory bound to this image will has been allocated lazily. */
		TransientAttachment = 0x00000040,
		/* Specifies that the image can be used to create an image view. */
		InputAttachment = 0x00000080
	};

	/* Defines additional parameters of an image. */
	enum class ImageCreateFlag
	{
		/* Specifies that the image will be backed using sparse memory binding. */
		SparseBinding = 0x00000001,
		/* Specified that the image can be partially back using sparse memory binding. */
		SparseResidency = 0x00000002,
		/* Specifies that the image will be backed using sparse memory binding with memory ranges that might also simultaneously be backing another image. */
		SparseAliased = 0x00000004,
		/* Specifies that the image can be used to create an image view with a different format from the image. */
		MutableFormat = 0x00000008,
		/* Specifies that the image can be used to create an cube image view or cube array image view. */
		CubeCompatible = 0x00000010
	};

	/* Defines the sample count limits. */
	enum class SampleCountFlag
	{
		/* No flags are set. */
		None = 0x00000000,
		/* Specifies an image with one sample per pixel. */
		Pixel1Bit = 0x00000001,
		/* Specifies an image with 2 samples per pixel. */
		Pixel2Bit = 0x00000002,
		/* Specifies an image with 4 samples per pixel. */
		Pixel4Bit = 0x00000004,
		/* Specifies an image with 8 samples per pixel. */
		Pixel8Bit = 0x00000008,
		/* Specifies an image with 16 samples per pixel. */
		Pixel16Bit = 0x00000010,
		/* Specifies an image with 32 samples per pixel. */
		Pixel32Bit = 0x00000020,
		/* Specifies an image with 64 samples per pixel. */
		Pixel64Bit = 0x00000040
	};

	/* Defines the capabilities of queues in a queue family. */
	enum class QueueFlag
	{
		/* No flags are set. */
		None = 0x00000000,
		/* Specifies that the queues in this queue family support graphics operations. */
		Graphics = 0x00000001,
		/* Specifies that the queues in this queue family support compute operations. */
		Compute = 0x00000002,
		/* Specifies that the queues in this queue family support transfer operations. */
		Transfer = 0x00000004,
		/* Specifies that the queues in this queue family support sparse memory management operations. */
		SparseBinding = 0x00000008,
		/* Specifies that the queues in this queue family support protected memory. */
		Protected = 0x00000010
	};

	/* Defines the properties of a memory heap. */
	enum class MemoryPropertyFlag
	{
		/* Specifies that the memory allocated with this type is most efficient for device access. */
		DeviceLocal = 0x00000001,
		/* Specified that the memory allocated with this type can be mapped for host access. */
		HostVisible = 0x00000002,
		/* Specifies that the host cache management command are not needed to flush host writes to the device. */
		HostCoherent = 0x00000004,
		/* Specifies that the memory allocated with this type is cached on the host. */
		HostCached = 0x00000008,
		/* Specifies that the memory type only allows device access to the memory. */
		LazilyAllocated = 0x00000010,
		/* Specifies that the memory type only allows device access to the memory and allows protected queue operations. */
		Protected = 0x00000020
	};

	/* Defines the attribute flags for heaps. */
	enum class MemoryHeapFlag
	{
		/* Specifies that the heap corresponds to device local memory. */
		DeviceLocal = 0x00000001,
		/* Specifies that in a logical device respresenting more than one physical devices, there is a per-physical device instance of the heap memory. */
		MultiInstance = 0x00000002
	};

	/* Defines the synchronization scopes of pipeline stages. */
	enum class PipelineStageFlag
	{
		/* Specifies the stage of the pipeline where any commands are initially received by the queue. */
		TopOfPipe = 0x00000001,
		/* Specifies the stage of the pipeline where Draw/DispatchIndirect data structures are consumed. */
		DrawIndirect = 0x00000002,
		/* Specifies the stage of the pipeline where vertex and index buffers are consumed. */
		VertexInput = 0x00000004,
		/* Specifies the vertex shader stage. */
		VertexShader = 0x00000008,
		/* Specifies the tessellation control shader stage. */
		TessellationControlShader = 0x00000010,
		/* Specfies the tessellation evaluation shader stage. */
		TessellationEvaluationShader = 0x00000020,
		/* Specifies the geometry shader stage. */
		GeometryShader = 0x00000040,
		/* Specifies the fragment shader stage. */
		FragmentShader = 0x00000080,
		/* Specifies the stage of the pipeline where early fragment tests are performed. */
		EarlyFragmentTests = 0x00000100,
		/* Specifies the stage of the pipeline where late fragment tests are performed. */
		LateFragmentTests = 0x00000200,
		/* Specifies the stage of the pipeline after blending where the final color values are output. */
		ColorAttachmentOutput = 0x00000400,
		/* Specifies the execution of a compute shader. */
		ComputeShader = 0x00000800,
		/* Specifies the execution of a copy command. */
		Transfer = 0x00001000,
		/* Specifies the final stage in the pipeline where operations generated by all commands complete execution. */
		BottomOfPipe = 0x00002000,
		/* Specifies a pseudo-stage indicating executin on the host of reads/writes of device memory. */
		Host = 0x00004000,
		/* Specifies the execution of all graphics pipeline stages. */
		AllGraphics = 0x00008000,
		/* This is equvalent to the logical OR of every other pipeline stage flag that is supported on the queue that it's used with. */
		AllCommands = 0x00010000
	};

	/* Defines the aspects of an image for purposes such as indentifying a subresource. */
	enum class ImageAspectFlag
	{
		/* Specifies the color aspect. */
		Color = 0x00000001,
		/* Specifies the depth aspect. */
		Depth = 0x00000002,
		/* Specifies the stencil aspect. */
		Stencil = 0x00000004,
		/* Specifies the metadata aspect. */
		MetaData = 0x00000008
	};

	/* Defines additional information about the sparse resource. */
	enum class SparseImageFormatFlag
	{
		/* Specifies that the image uses a single mip tail region for all array layers. */
		SingleMipTail = 0x00000001,
		/* Specifies that the first mip level whose dimensions are not integer multiples of the corresponding dimension of the sparse image block begins the mip tail region. */
		AlignedMipSize = 0x00000002,
		/* Specifies that the images uses non-standard sparse image block dimensions. */
		NonStandardBlockSize = 0x00000004
	};

	/* Defines the usage of a sparse memory binding operation. */
	enum class SparseMemoryBindFlag
	{
		/* Specifies that the memory being bound is only for the metadata aspect. */
		MetaData = 0x00000001
	};

	/* Defines the initial state and behaviour of a fence. */
	enum class FenceCreateFlag
	{
		/* No flags were set. */
		None = 0x00000000,
		/* Specifies that the fence object is created in the signaled state. */
		Signaled = 0x00000001
	};

	/* Defines the queried pipeline statistics. */
	enum class QueryPipelineStatisticFlag
	{
		/* No flags were set. */
		None = 0x00000000,
		/* Specifies that queries managed by the pool will count the number of vertices processed by the input assembly stage. */
		InputAssemblyVertices = 0x00000001,
		/* Specifies that queries managed by the pool will count the number of primitives processed by the input assembly stage. */
		InputAssemblyprimitives = 0x00000002,
		/* Specifies that queries managed by the pool will count the number of vertex shader invocations.  */
		VertexShaderInvocations = 0x00000004,
		/* Specifies that queries managed by the pool will count the number of geometry shader invocations. */
		GeometryShaderInvocations = 0x00000008,
		/* Specifies that queries managed by the pool will count the number of primitives generated by the geometry shader invocations. */
		GeometryShaderPrimitives = 0x00000010,
		/* Specifies that queries managed by the pool will count the number of primitives processed by the primitive clipping stage. */
		ClippingInvocations = 0x00000020,
		/* Specifies that queries managed by the pool will count the number of primitives output by the primitive clipping stage. */
		ClippingPrimitives = 0x00000040,
		/* Specifies that queries managed by the pool will count the number of fragment shader invocations. */
		FragmentShaderInvocations = 0x00000080,
		/* Specifies that queries managed by the pool will count the number of patches processed by the tessellation control shader. */
		TessellationControlShaderPatches = 0x00000100,
		/* Specifies that queries managed by the pool will count the number of invocations of the tessellation evaluation shader. */
		TessellationEvaluationShaderInvocations = 0x00000200,
		/* Specifies that queries managed by the pool will count the number of compute shader invocations. */
		ComputeShaderInvocations = 0x00000400
	};

	/* Defines how and when result are returned. */
	enum class QueryResultFlag
	{
		/* Specifies the result will be written as an array of 64-bit unsigned values. */
		Result64Bit = 0x00000001,
		/* Specifies that Vulkan will wait for each query's status to become available before retrieving its results. */
		Wait = 0x00000002,
		/* Specifies that the availabilty status accompanies the results. */
		WitchAvailability = 0x00000004,
		/* Specifies that returning partial results is acceptable. */
		Partial = 0x00000008
	};

	/* Defines additional parameters of a buffer. */
	enum class BufferCreateFlag
	{
		/* Specifies that the buffer will be backed using sparse memory binding. */
		SparseBinding = 0x00000001,
		/* Specifies that the buffer can be partially backed using sparse memory binding. */
		SparseResidency = 0x00000002,
		/* Specifies that the buffer will be backed using sparse memory binding with memory ranges that might also simulaneously be backing another buffer. */
		Aliased = 0x00000004,
		/* Specifies that the buffer is a protected buffer. */
		Protected = 0x00000008
	};

	/* Defines the usage behaviour of a buffer. */
	enum class BufferUsageFlag
	{
		/* Specifies that the buffer can be used as the source of a transfer command. */
		TransferSrc = 0x00000001,
		/* Specifies that the buffer can be used as the destination of a transfer command. */
		TransferDst = 0x00000002,
		/* Specifies that the buffer can be used to create a uniform texel buffer view. */
		UniformTexelBuffer = 0x00000004,
		/* Specifies that the buffer can be used to create a storage texel buffer view. */
		StorageTexelBuffer = 0x00000008,
		/* Specifies that the buffer can be used to create a uniform buffer view. */
		UniformBuffer = 0x00000010,
		/* Specifies that the buffer can be used to create a storage buffer view. */
		StorageBuffer = 0x00000020,
		/* Specifies that the buffer is suitable for passing as the buffer parameter to CmdBindIndexBuffer. */
		IndexBuffer = 0x00000040,
		/* Specifies that the buffer is suitable for passing as the buffer parameter to CmdBindVertexBuffer. */
		VertexBuffer = 0x00000080,
		/* Specified that the buffer is suitable for passing as a buffer parameter to indirect drawing commands. */
		IndirectBuffer = 0x00000100
	};

	/* Defines how a pipeline is created. */
	enum class PipelineCreateFlag
	{
		/* No flags where set. */
		None = 0x00000000,
		/* Specifies that the pipeline will not be optimized. */
		DisableOptimization = 0x00000001,
		/* Specifies that the pipeline to be created is allowed to be a parent of a pipeline. */
		AllowDerivatives = 0x00000002,
		/* Specifies that the pipeline to be created will be a child of a previously created parent pipeline. */
		Derivative = 0x00000004,
		/* Specifies that any shader input variables decorated as ViewIndex will be assigned values as if they were decorated as DeviceIndex. */
		ViewIndexFromDeviceIndex = 0x00000008,
		/* Specifies that a compute pipeline can be used with CmdDispatchBase with a non-zero base workgroup. */
		DispatchBase = 0x00000010
	};

	/* Defines the shader stages. */
	enum class ShaderStageFlag
	{
		/* Shader stage is unknown. */
		Unknown = 0x00000000,
		/* Specifies the vertex stage. */
		Vertex = 0x00000001,
		/* Specifies the tessellation control stage. */
		TessellationControl = 0x00000002,
		/* Specifies the tessellation evaluation stage. */
		TessellationEvaluation = 0x00000004,
		/* Specifies the geometry stage. */
		Geometry = 0x00000008,
		/* Specifies the fragment stage. */
		Fragment = 0x00000010,
		/* Specifies the compute stage. */
		Compute = 0x00000020,
		/* This is a combination of bits used to specify all graphics stages. */
		AllGraphics = 0x0000001F,
		/* This is a combination of bits used to specify all shader stages supported by the device. */
		All = 0x7FFFFFFF
	};

	/* Defines the types of triangle culling. */
	enum class CullModeFlag
	{
		/* Specified that no triangle are discarded. */
		None = 0,
		/* Specified that front-facing triangles are discarded. */
		Front = 0x00000001,
		/* Specifies that back-facing triangles are discarded. */
		Back = 0x00000002,
		/* Specifies that all triangles are discarded. */
		FrontAndBack = 0x00000003
	};

	/* Defines the color components that are writen to the framebuffer. */
	enum class ColorComponentFlag
	{
		/* Specifies that the Red value is written to the color attachment. */
		R = 0x00000001,
		/* Specifies that the Greed value is written to the color attachment. */
		G = 0x00000002,
		/* Specifies that the Blue value is written to the color attachment. */
		B = 0x00000004,
		/* Specifies that the Alpha value is written to the color attachment. */
		A = 0x00000008,
		/* Specifies that the red green and blue values are written to the color attachment. */
		RGB = R | G | B,
		/* Specifies that all components are writen to the color attachment. */
		RGBA = R | G | B | A
	};

	/* Defines the operations available on a descriptor pool. */
	enum class DescriptorPoolCreateFlag
	{
		/* Specifies that the descriptor sets can return their individual allocations to the pool. */
		FreeDescriptorSet = 0x00000001
	};

	/* Defines additional properties of attachments. */
	enum class AttachmentDescriptionFlag
	{
		/* No flags where set. */
		None = 0x00000001,
		/* Specifies that the attachment aliases the same device memory as other attachments. */
		MayAlias = 0x00000001
	};

	/* Defines the memory access types that will participate in a memory dependency. */
	enum class AccessFlag
	{
		/* No flags where set. */
		None = 0x00000000,
		/* Specifies read access to an indirect command structure read as part of an indirect drawing or dispatch command. */
		InirectCommandRead = 0x00000001,
		/* Specifies read access to an index buffer as part of an indexed drawing command. */
		IndexRead = 0x00000002,
		/* Specified read access to a vertex buffer as part of a drawing command. */
		VeretxAttributeRead = 0x00000004,
		/* Specifies read access to a uniform buffer. */
		UniformRead = 0x00000008,
		/* Specifies read access to an input attachment within a render pass during fragment shading. */
		InputAttachmentRead = 0x00000010,
		/* Specifies read access to a storage buffer, uniform texel buffer, storage buffer, sapled image or storage image. */
		ShaderRead = 0x00000020,
		/* Specifies write access to a storage buffer storage, storage texel buffer or storage image. */
		ShaderWrite = 0x00000040,
		/* Specifies read access to a color attachment. */
		ColorAttachmentRead = 0x00000080,
		/* Specifies write access to a color attachment. */
		ColorAttachmentWrite = 0x00000100,
		/* Specifies read access to a depth/stencil attachment. */
		DepthStencilAttachmentRead = 0x00000200,
		/* Specifies write access to a depth/stencil attachment. */
		DepthStencilAttachmentWrite = 0x00000400,
		/* Specifies read access to an image or buffer in a copy operation. */
		TransferRead = 0x00000800,
		/* Specifies write access to an image or buffer in a clear or copy operation. */
		TransferWrite = 0x00001000,
		/* Specifies read access by a host operation. */
		HostRead = 0x00002000,
		/* Specifies write access by a host operation. */
		HostWrite = 0x00004000,
		/* specifies read access via non-specific entities. */
		MemoryRead = 0x00008000,
		/* Specifies write access via non-specific entities. */
		memoryWrite = 0x00010000,
	};

	/* Defines how execution and memory dependencies are formed. */
	enum class DependencyFlag
	{
		/* No flags where set. */
		None = 0x00000001,
		/* Specifies that dependencies will be framebuffer-local. */
		ByRegion = 0x00000001,
		/* Specifies that dependencies are non-device-local. */
		DeviceGroup = 0x00000004,
		/* Specifies that a subpass has more that one view. */
		Local = 0x00000002
	};

	/* Defines the usage behaviour for a command pool. */
	enum class CommandPoolCreateFlag
	{
		/* No flags have been set. */
		None = 0x00000000,
		/* Specifies hat command buffers allocated from the pool will be short-lived. */
		Transient = 0x00000001,
		/* Allows any command buffer allocated from a pool to be individually reset. */
		ResetCommandBuffer = 0x00000002,
		/* Specifies that command buffers allocated from the pool are protected command buffers. */
		Protected = 0x00000004
	};

	/* Defines the controlling behaviour of a command pool reset. */
	enum class CommandPoolResetFlag
	{
		/* Specifies that resetting a command pool recycles all of the resources from the command pool. */
		ReleaseResources = 0x00000001
	};

	/* Defines the usage behaviour for command buffers. */
	enum class CommandBufferUsageFlag
	{
		/* Specifies that each recording of the command buffer will only be submitted once. */
		OneTimeSubmit = 0x00000001,
		/* Specifies that a secondary command buffer is considered to be entirely inside a render pass. */
		RenderPassContinue = 0x00000002,
		/* Specifies that a command buffer can be resubmitted to a queue while it's in the pending state. */
		SimultaneousUse = 0x00000004
	};

	/* Defines the specific constraints on a query. */
	enum class QueryControlFlag
	{
		/* No flags where set. */
		None = 0x00000000,
		/* Specifies the precision of occlusion queries. */
		Precise = 0x00000001
	};

	/* Defines the behavior of a command buffer reset. */
	enum class CommandBufferResetFlag
	{
		/* Specifies that most or all memory resources currently owned by the command buffer should be returned to the parent command pool. */
		ReleaseResrouces = 0x00000001
	};

	/* Defines sets of stencil state for which to update the compare mask. */
	enum class StencilFaceFlag
	{
		/* Specifies that only the front set of stencil state is updated. */
		Front = 0x00000001,
		/* Specifies that only the back set of stencil state is updated. */
		Back = 0x00000002,
		/* Specified that both sets of stencil state are updated. */
		FrontAndBack = 0x00000003
	};

	/* Defines behaviour of the queue. */
	enum class DeviceQueueCreateFlag
	{
		/* No flags are set. */
		None = 0x00000000,
		/* Specifies that the device queue is a protected-capable queue. */
		ProtectedBit = 0x00000001
	};

	/* Defines surface presentation transformations. */
	enum class SurfaceTransformFlag
	{
		/* No transformation is applied when presenting. */
		Identity = 0x00000001,
		/* The image is rotated 90 degrees clockwise. */
		Rotate90 = 0x00000002,
		/* The image is rotated 180 degrees clockwise. */
		Rotate180 = 0x00000004,
		/* The image is rotated 270 degrees clockwise. */
		Rotate270 = 0x00000008,
		/* The image is mirrored horizontally. */
		MirrorHorizontal = 0x00000010,
		/* The image is mirrored horizontally, then rotated 90 degrees clockwise. */
		MirrorHorizontalRotate90 = 0x00000020,
		/* The image is mirrored horizontally, then rotated 180 degrees clockwise. */
		MirrorHorizontalRotate180 = 0x00000040,
		/* The image is mirrored horizontally, then rotated 270 degrees clockwise. */
		MirrorHorizontalRotate270 = 0x00000080,
		/* The presentation transform is not specified. */
		Inherit = 0x00000100
	};

	/* Defines how the alpha channel in an image should be handled. */
	enum class CompositeAlphaFlag
	{
		/* The alpha channel (if it exists) is ignored. */
		Opaque = 0x00000001,
		/* The alpha channel (if it exists) of the images are respected. The non-alpha channels of the image are expected to already be multipled by the alpha. */
		PreMultiplied = 0x00000002,
		/* The alpha channel (if it exists) of the images are respected. The non-alpha channels of the image are not expected to already be mutiplied by the alpha. */
		PostMultiplied = 0x00000004,
		/* The alpha composition is handled by the native window system commands. */
		Inherit = 0x00000008
	};

	/* Defines different types of color space. */
	enum class ColorSpace
	{
		/* Non-linear sRGB color space. */
		SRGB = 0,
	};

	/* Defines types of image present modes. */
	enum class PresentMode
	{
		/* The presentation engine doesn't wait for a vertical blanking period to update the current image. */
		Immediate = 0,
		/* The presentation engine waits for a vertical blanking period to update the current image. Only the newest presentation is kept in the queue. */
		MailBox = 1,
		/* The presentation engine waits for a vertical blanking period to update the current image. The new entry is added to the back of the queue. */
		FiFo = 2,
		/* The presentation engine generally waits for a vertical blanking period to update the current image. The new entry is added to the back of the queue. */
		FiFoRelaxed = 3
	};

	/* Defines the usage of a subpass. */
	enum class SubpassDescriptionFlag
	{
		/* No flags were set. */
		None = 0x00000000,
		/* Shaders compiled for this subpass write the attributes for all views in a single invocation of each vertex processing stage. */
		PerViewAttributes = 0x00000001,
		/* Shaders compiled for this subpass use per-view positions which only differ in value in the x component. */
		PerViewPositionXOnly = 0x00000002
	};

	/* Defines the types of message severities in the debug extension. */
	enum class DebugUtilsMessageSeverityFlag
	{
		/* No flags were set. */
		None = 0x00000000,
		/* Indicates all diagnostic messages. */
		Verbose = 0x00000001,
		/* Indicates an informational message. */
		Info = 0x00000010,
		/* Indicates that the use of Vulkan may expose an app bug. */
		Warning = 0x00000100,
		/* Indicates that the app is causes undefined results. */
		Error = 0x00001000,
		/* All flags are set. */
		All = Verbose | Info | Warning | Error,
	};

	/* Defines the type of message event. */
	enum class DebugUtilsMessageTypeFlag
	{
		/* No flags were set. */
		None = 0x00000000,
		/* Indicates a general event. */
		General = 0x00000001,
		/* Indicated an event during validation against the Vulkan specification. */
		Validation = 0x00000002,
		/* Indicates a potential non-optimal use of Vulkan. */
		Performance = 0x00000004,
		/* All flags are set. */
		All = General | Validation| Performance,
	};

	/* Defines handler types in Vulkan. */
	enum class ObjectType 
	{
		Unknown = 0,
		Instance = 1,
		PhysicalDevice = 2,
		Device = 3,
		Queue = 4,
		Semaphore = 5,
		CommandBuffer = 6,
		Fence = 7,
		DeviceMemory = 8,
		Buffer = 9,
		Image = 10,
		Event = 11,
		QueryPool = 12,
		BufferView = 13,
		ImageView = 14,
		ShaderModule = 15,
		PipelineCache = 16,
		PipelineLayout = 17,
		RenderPass = 18,
		Pipeline = 19,
		DescriptionSetLayout = 20,
		Sampler = 21,
		DescriptorPool = 22,
		DescriptorSet = 23,
		Framebuffer = 24,
		CommandPool = 25,
		SamplerYCBCRCOnversion = 1000156000,
		DescriptorUpdateTemplate = 1000085000,
		Surface = 1000000000,
		Swapchain = 1000001000,
		Display = 1000002000,
		DisplayMode = 1000002001,
		DebugReportCallback = 1000011000,
		DebugUtilsMessanger = 1000128000,
		ValidationCache = 1000160000,
	};

	/* Appends the flag bits of an image usage flag. */
	_Check_return_ inline ImageUsageFlag operator |(_In_ ImageUsageFlag a, _In_ ImageUsageFlag b)
	{
		return _CrtEnumBitOr(a, b);
	}

	inline void ValidateVkApiResult(_In_ VkApiResult result, _In_ string procedure)
	{
		/* Remove PFN_ prefix and KHR suffix. */
		procedure.remove({ "PFN_", "KHR" });

		const char *code = "";
		bool raise = true;

		/* Convert result to string and check whether it needs to raise. */
		switch (result)
		{
		case Pu::VkApiResult::Success:
			return;
		case Pu::VkApiResult::NotReady:
			raise = false;
			code = "Not ready";
			break;
		case Pu::VkApiResult::Timeout:
			raise = false;
			code = "Timeout";
			break;
		case Pu::VkApiResult::EventSet:
			raise = false;
			code = "Event signaled";
			break;
		case Pu::VkApiResult::EventReset:
			raise = false;
			code = "Event unsignaled";
			break;
		case Pu::VkApiResult::Incomplete:
			raise = false;
			code = "Incomplete";
			break;
		case Pu::VkApiResult::SuboptimalKhr:
			raise = false;
			code = "Suboptimal swapchain";
			break;
		case Pu::VkApiResult::HostOutOfMemory:
			code = "Host out of memory";
			break;
		case Pu::VkApiResult::OutOfDeviceMemory:
			code = "Out of device memory";
			break;
		case Pu::VkApiResult::InitializationFailed:
			code = "Initialization failed";
			break;
		case Pu::VkApiResult::DeviceLost:
			code = "Device lost";
			break;
		case Pu::VkApiResult::MemoryMapFailed:
			code = "Memory mapping failed";
			break;
		case Pu::VkApiResult::LayerNotPresent:
			code = "Layer not present";
			break;
		case Pu::VkApiResult::ExtensionNotPresent:
			code = "Extension not present";
			break;
		case Pu::VkApiResult::FeatureNotPresent:
			code = "Feature not present";
			break;
		case Pu::VkApiResult::IncompatibleDriver:
			code = "Incompatible driver";
			break;
		case Pu::VkApiResult::TooManyObjects:
			code = "Too many objects";
			break;
		case Pu::VkApiResult::FormatNotSupported:
			code = "Format not supported";
			break;
		case Pu::VkApiResult::FragmentedPool:
			code = "Fragmented pool";
			break;
		case Pu::VkApiResult::OutOfPoolMemory:
			code = "Out of pool memory";
			break;
		case Pu::VkApiResult::InvalidExternalHandle:
			code = "Invalid external handle";
			break;
		case Pu::VkApiResult::SurfaceLostKhr:
			code = "Surface lost";
			break;
		case Pu::VkApiResult::NativeWindowInUseKhr:
			code = "Native window in use";
			break;
		case Pu::VkApiResult::OutOfDateKhr:
			code = "Out of date";
			break;
		case Pu::VkApiResult::IncompatibleDisplayKhr:
			code = "Incompatible display";
			break;
		case Pu::VkApiResult::ValidationFailedExt:
			code = "Validation failed";
			break;
		case Pu::VkApiResult::InvalidShaderNv:
			code = "Invalid shader";
			break;
		case Pu::VkApiResult::FragmentationExt:
			code = "Fragmentation";
			break;
		case Pu::VkApiResult::NotPermittedExt:
			code = "Not premitted";
			break;
		}

		/* Log findings if needed. */
		if (raise) Log::Fatal("Procedure %s failed with error code '%s'!", procedure.c_str(), code);
		else Log::Warning("Procedure %s produced non-success code '%s'!", procedure.c_str(), code);
	}

	/* Converts a shader stage flag to string. */
	_Check_return_ inline const char* to_string(_In_ ShaderStageFlag stage)
	{
		switch (stage)
		{
		case ShaderStageFlag::Vertex:
			return "Vertex";
		case ShaderStageFlag::TessellationControl:
			return "Tessellation control";
		case ShaderStageFlag::TessellationEvaluation:
			return "Tessellation evaluation";
		case ShaderStageFlag::Geometry:
			return "Geometry";
		case ShaderStageFlag::Fragment:
			return "Fragment";
		case ShaderStageFlag::Compute:
			return "Compute";
		default:
			return "Unknown";
		}
	}
}