#pragma once
#include "Core/Platform/Windows/Windows.h"
#include "VulkanFunctions.h"
#include "Config.h"

namespace Pu
{
	/* Defines application info. */
	struct ApplicationInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* A null terminated UTF-8 string containing the name of the application or nullptr. */
		const char *ApplicationName;
		/* Defines the developer-supplied version number of the application. */
		uint32 ApplicationVersion;
		/* A null terminated UTF-8 string containing the name of the engine (if any) or nullptr. */
		const char *EngineName;
		/* Defines the developer-supplied version number of the engine. */
		uint32 EngineVersion;
		/* The highest version of the Vulkan API that the application is designed to use. */
		uint32 ApiVersion;

		/* Initializes an empty instance of an application information object. */
		ApplicationInfo(void)
			: ApplicationInfo(nullptr, 0, 0, 0, nullptr, 0, 0, 0)
		{}

		/* Initializes a new instance of an application info object. */
		ApplicationInfo(_In_ const char *applicationName, _In_ uint32 appMajor, _In_ uint32 appMinor, _In_ uint32 appPatch,
			_In_ const char *engineName, _In_ uint32 engineMajor, _In_ uint32 engineMinor, _In_ uint32 enginePatch)
			: Type(StructureType::ApplicationInfo), Next(nullptr),
			ApplicationName(applicationName), ApplicationVersion(makeVersion(appMajor, appMinor, appPatch)),
			EngineName(engineName), EngineVersion(makeVersion(engineMajor, engineMinor, enginePatch)),
			ApiVersion(VulkanVersion)
		{}
	};

	/* Defines parameters of a newly created instance. */
	struct InstanceCreateInfo
	{
	public:
		/* The type of this structure  */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved for future use. */
		Flags Flags;
		/* Pointer to information about the application or nullptr. */
		const ApplicationInfo *ApplicationInfo;
		/* The number of global layers to enable. */
		uint32 EnabledLayerCount;
		/* A null terminated UTF-8 string with size of EnabledLayerCount that contains the names of layers to enabled for the created instance. */
		const char * const*EnabledLayerNames;
		/* The number of global extensions to enable. */
		uint32 EnabledExtensionCount;
		/* A null terminated UTF-8 string with size of EnabledExtensionCount that contains the names of extensions to enabled for the created instance. */
		const char * const*EnabledExtensionNames;

		/* Initializes an empty instance of an instance create information object.  */
		InstanceCreateInfo(void)
			: Type(StructureType::InstanceCreateInfo), Next(nullptr),
			Flags(0), ApplicationInfo(nullptr),
			EnabledLayerCount(0), EnabledLayerNames(nullptr),
			EnabledExtensionCount(0), EnabledExtensionNames(nullptr)
		{}

		/* Initializes a new instance of a instance create information object. */
		InstanceCreateInfo(_In_ const Pu::ApplicationInfo &applicationInfo, _In_ const vector<const char*> &extensions)
			: Type(StructureType::InstanceCreateInfo), Next(nullptr),
			Flags(0), ApplicationInfo(&applicationInfo),
			EnabledLayerCount(0), EnabledLayerNames(nullptr),
			EnabledExtensionCount(static_cast<uint32>(extensions.size())), EnabledExtensionNames(extensions.data())
		{}
	};

	/* Defines an object that contains callback function pointers for memory allocation. */
	struct AllocationCallbacks
	{
	public:
		/* Defines user-interpreted data to be send to the callbacks. */
		void *UserData;
		/* Pointer to an application-defined memory allocation function. */
		AllocationFunction Allocation;
		/* Pointer to an application-defined memory reallocation function. */
		ReallocationFunction Reallocation;
		/* Pointer to an application-defined memory free function. */
		FreeFunction Free;
		/* Pointer to an application-defined internal memory allocation function. */
		InternalAllocationNotification InternalAllocation;
		/* Pointer to an application-defined internal memory free function. */
		InternalFreeNotification InternalFree;

		/* Initializes an empty instance of an allocation callbacks object. */
		AllocationCallbacks(void)
			: AllocationCallbacks(nullptr, nullptr, nullptr)
		{}

		/* Initializes a new instance of an allocation callbacks object. */
		AllocationCallbacks(_In_ AllocationFunction allocation, _In_ ReallocationFunction reallocation, _In_ FreeFunction free)
			: UserData(nullptr),
			Allocation(allocation), Reallocation(reallocation), Free(free),
			InternalAllocation(nullptr), InternalFree(nullptr)
		{}

		/* Checks if the structure is defined in a correct way. */
		_Check_return_ inline bool IsValid(void) const
		{
			return (Allocation && Reallocation && Free) && ((InternalAllocation && InternalFree) || (!InternalAllocation && !InternalFree));
		}
	};

	/* Defines the fine-grained features that can be supported by an implementation. */
	struct PhysicalDeviceFeatures
	{
	public:
		/* Specifies that accesses to buffers are bound-checked against the range of the buffer desriptor. */
		Bool32 RobustBufferAccess;
		/* Specifies the full 32-bit range of indices is supported for indexed draw calls. */
		Bool32 FullDrawIndexUint32;
		/* Specifies whether image views with a ViewType of CubeArray can be created. */
		Bool32 ImageCubeArray;
		/* Specifies whether the PipelineColorBlendAttachmentState settgins are controlled independently per-attachment. */
		Bool32 IndependentBlend;
		/* Specifies whether geometry shaders are supported. */
		Bool32 GeometryShader;
		/* Specifies whether trssellation control and evaluation shaders are supported. */
		Bool32 TessellationShader;
		/* Specifies whether Sample Shading and multisample interpolation are supported. */
		Bool32 SampleRateShading;
		/* Specifies whether blend operations which take two sources are supported. */
		Bool32 DualSrcBlend;
		/* Specifies whether logic operatrions are supported. */
		Bool32 LogicOp;
		/* Specifies whether multiple draw indirect is supported. */
		Bool32 MultiDrawIndirect;
		/* Specifies whether indirect draw calls support FirstInstance parameters. */
		Bool32 DrawIndirectFirstInstance;
		/* Specifies whether depth clamping is supported. */
		Bool32 DepthClamp;
		/* Specifies whether depth bias clamping is supported. */
		Bool32 DepthBiasClamp;
		/* Specifies whether point and wireframe fill modes are supported. */
		Bool32 FillModeNonSolid;
		/* Specifies whether depth bounds tests are supported. */
		Bool32 DepthBounds;
		/* Specifies whether lines with width other than 1.0 are supported. */
		Bool32 WideLines;
		/* Specifies whether points with sie greater than 1.0f are supported. */
		Bool32 LargePoints;
		/* Specifies whether the implementation is able to replace the alpha value of color fragment output from the fragment shader. */
		Bool32 AlphaToOne;
		/* Specifies whether more than one viewport is supported. */
		Bool32 MultiViewport;
		/* Specifies whether anisotropic filtering is supported. */
		Bool32 SamplerAnisotropy;
		/* Specifies whether all of the ETC2 and EAC compressed texture formats are supported. */
		Bool32 TextureCompressionETC2;
		/* Specifies whether all of the ASTC_LDR compressed texture formats are supported. */
		Bool32 TextureCompressionASTC_LDR;
		/* Specifies whether all of the BC compressed texture formats are supported. */
		Bool32 TextureCompressionBC;
		/* Specifies whether occlusion queries returning actual sample counts are supported. */
		Bool32 OcclusionQueryPrecise;
		/* Specifies whether the pipeline staticstics queries are supported. */
		Bool32 PipelineStatisticsQuery;
		/* Specifies whether storage buffers and images support stores and atomic operations in the vertex, tessellation and geometry shader stages. */
		Bool32 VertexPipelineStoresAndAtomics;
		/* Specifies whether storage buffers and images support stores and atomic operations in the fragment shader stage. */
		Bool32 FragmentStoresAndAtomics;
		/* Specifies whether the PointSize built-in decoration is available in the tessellation control, tessellation evaluation and geometry shader stages. */
		Bool32 ShaderTessellationAndGeometryPointSize;
		/* Specifies whether the extended set of images gather instructions are available in shader code. */
		Bool32 ShaderImageGatherExtended;
		/* Specifies whether all the extended storage image formats are available in shader code. */
		Bool32 ShaderStorageImageExtendedFormats;
		/* Specifies whether multisampled storage images are supported. */
		Bool32 ShaderStorageImageMultisample;
		/* Specifies whether storage images require a format qualifier to be specified when reading from storage images. */
		Bool32 ShaderStorageImageReadWithoutFormat;
		/* Specifies whether storage images require a format qualifier to be specified when writing to storage images. */
		Bool32 ShaderStorageImageWriteWithoutFormat;
		/* Specifies whether arrays of uniform buffers can be indexed by dynamically uniform integer expressions in shader code. */
		Bool32 ShaderUniformBufferArrayDynamicIndexing;
		/* Specifies whether ararys of samplers or sampled images can be indexed by dynamically uniform integer expressions in shader code. */
		Bool32 ShaderSampledImageArrayDynamicIndexing;
		/* Specifies whether arrays of storage buffers can be indexed by dynamically uniform integer expressions in shader code. */
		Bool32 ShaderStorageBufferArrayDynamicIndexing;
		/* Specifies whether arrays of storage images can be indexed by dynamically uniform interger expressions in shader code. */
		Bool32 ShaderStorageImageArrayDynamicIndexing;
		/* Specifies whether clip distances are supported in shader code. */
		Bool32 ShaderClipDistance;
		/* Specifies whether cull distances are supported in shader code. */
		Bool32 ShaderCullDistance;
		/* Specifies whether doubles are supported in shader code. */
		Bool32 ShaderFloat64;
		/* Specifies whether longs are supported in shader code. */
		Bool32 ShaderInt64;
		/* Defines whether shorts are supported in shader code. */
		Bool32 ShaderInt16;
		/* Specifies whether image operations that return residency information are supported in shader code. */
		Bool32 ShaderResourceResidency;
		/* Specifies whether image operations that specify the minimum resource LOD are supported in shader code. */
		Bool32 ShaderResourceMinLod;
		/* Specifies whether resource memory can be managed at opaque sparse block level instead of at the object level. */
		Bool32 SparseBinding;
		/* Specifies whether the device can access partially resident buffers. */
		Bool32 SparseResidencyBuffer;
		/* Specifies whether the deivce can access partially resident 2D images with 1 sample per pixel. */
		Bool32 SparseResidencyImage2D;
		/* Specifies whether the device can access partially resident 3D images. */
		Bool32 SparseResidencyImage3D;
		/* Specifies whether the physical device can access partially resident 2D images with 2 samples per pixel. */
		Bool32 SparseResidency2Samples;
		/* Specifies whether the physical device can access partially resident 2D images with 4 samples per pixel. */
		Bool32 SparseResidency4Samples;
		/* Specifies whether the physical device can access partially resident 2D images with 8 samples per pixel. */
		Bool32 SparseResidency8Samples;
		/* Specifies whether the physical device can access partially resident 2D images with 16 samples per pixel. */
		Bool32 SparseResidency16Samples;
		/* specifies whether the physical device can correctly access data aliased into multiple locations. */
		Bool32 SparseResidencyAliased;
		/* Specifies whether all pipelines that will be bound to a command buffer during a subpass with no attachments must have the same value for PipelineMultisampleStateCreateInfo::RasterizationSamples. */
		Bool32 VariableMultisampleRate;
		/* Specifies whether a secondary command buffer may be executed while a query is active. */
		Bool32 InheritedQueries;

		/* Initializes an empty instance of a physical device features object. */
		PhysicalDeviceFeatures(void)
			: RobustBufferAccess(false), FullDrawIndexUint32(false), ImageCubeArray(false), IndependentBlend(false), GeometryShader(false),
			TessellationShader(false), SampleRateShading(false), DualSrcBlend(false), LogicOp(false), MultiDrawIndirect(false), DrawIndirectFirstInstance(false),
			DepthClamp(false), DepthBiasClamp(false), FillModeNonSolid(false), DepthBounds(false), WideLines(false), LargePoints(false), AlphaToOne(false),
			MultiViewport(false), SamplerAnisotropy(false), TextureCompressionETC2(false), TextureCompressionASTC_LDR(false), TextureCompressionBC(false),
			OcclusionQueryPrecise(false), PipelineStatisticsQuery(false), VertexPipelineStoresAndAtomics(false), FragmentStoresAndAtomics(false),
			ShaderTessellationAndGeometryPointSize(false), ShaderImageGatherExtended(false), ShaderStorageImageExtendedFormats(false),
			ShaderStorageImageMultisample(false), ShaderStorageImageReadWithoutFormat(false), ShaderStorageImageWriteWithoutFormat(false),
			ShaderUniformBufferArrayDynamicIndexing(false), ShaderSampledImageArrayDynamicIndexing(false), ShaderStorageBufferArrayDynamicIndexing(false),
			ShaderStorageImageArrayDynamicIndexing(false), ShaderClipDistance(false), ShaderCullDistance(false), ShaderFloat64(false),
			ShaderInt64(false), ShaderInt16(false), ShaderResourceResidency(false), ShaderResourceMinLod(false), SparseBinding(false),
			SparseResidencyBuffer(false), SparseResidencyImage2D(false), SparseResidencyImage3D(false), SparseResidency2Samples(false),
			SparseResidency4Samples(false), SparseResidency8Samples(false), SparseResidency16Samples(false), SparseResidencyAliased(false),
			VariableMultisampleRate(false), InheritedQueries(false)
		{}
	};

	/* Defines image format properties. */
	struct FormatProperties
	{
	public:
		/* Specifies the features supported by images created with a tiling parameter of Linear. */
		FormatFeatureFlag LinearTilingFeatures;
		/* Specifies the features supported by images created with a tiling parameter of Optimal. */
		FormatFeatureFlag OptimalTilingFeatures;
		/* Specifies the features supported by buffers. */
		FormatFeatureFlag BufferFeatures;

		/* Initializes an empty instance of a format properties object. */
		FormatProperties(void)
			: LinearTilingFeatures(FormatFeatureFlag::None), OptimalTilingFeatures(FormatFeatureFlag::None), BufferFeatures(FormatFeatureFlag::None)
		{}
	};

	/* Defines a two-dimensional extent. */
	struct Extent2D
	{
	public:
		/* The width of the extent. */
		uint32 Width;
		/* The height of the extent. */
		uint32 Height;

		/* Initializes an empty instance of an 2D extent object. */
		Extent2D(void)
			: Width(0), Height(0)
		{}

		/* Initializes a new instace of a 2D extent object. */
		Extent2D(_In_ uint32 width, _In_ uint32 height)
			: Width(width), Height(height)
		{}

		/* Checks whether the left side is smaller than the right side. */
		_Check_return_ inline bool operator <(_In_ const Extent2D &other) const
		{
			return Width < other.Width && Height < other.Height;
		}
	};

	/* Defines a three-dimensional extent. */
	struct Extent3D
	{
	public:
		/* The width of the extent. */
		uint32 Width;
		/* The height of the extent. */
		uint32 Height;
		/* The depth of the extent. */
		uint32 Depth;

		/* Initializes an empty instance of an 3D extent object. */
		Extent3D(void)
			: Width(0), Height(0), Depth(0)
		{}

		/* Initializes a new instace of a 3D extent object. */
		Extent3D(_In_ uint32 width, _In_ uint32 height, _In_ uint32 depth)
			: Width(width), Height(height), Depth(depth)
		{}

		/* Initializes a new instance of a 3D extent from a 2D extend and a depth value. */
		Extent3D(_In_ Extent2D widthHeight, _In_ uint32 depth)
			: Width(widthHeight.Width), Height(widthHeight.Height), Depth(depth)
		{}

		/* Gets the width and height components of the extent3D as a extent2D. */
		_Check_return_ inline Extent2D To2D(void) const
		{
			return Extent2D(Width, Height);
		}

		/* Checks whether the left side is smaller than the right side in any dimension. */
		_Check_return_ inline bool operator <(_In_ const Extent3D &other) const
		{
			return Width < other.Width || Height < other.Height || Depth < other.Depth;
		}
	};

	/* Defines image format properties. */
	struct ImageFormatProperties
	{
	public:
		/* Specifies the maximum image dimensions. */
		Extent3D MaxExtent;
		/* Specifies the maximum number of mipmap levels. */
		uint32 MaxMipLevels;
		/* Specifies the maximum number of array layers. */
		uint32 MaxArrayLayers;
		/* Specifies all supported sample counts for this image. */
		SampleCountFlag SampleCounts;
		/* Specifies an upper bounds on the total image size in bytes. */
		DeviceSize MaxResourceSize;

		/* Initializes an empty instance of an image format properties object. */
		ImageFormatProperties(void)
			: MaxExtent(), MaxMipLevels(0), MaxArrayLayers(0), SampleCounts(SampleCountFlag::None), MaxResourceSize(0)
		{}
	};

	/* Defines the implementation-dependent physical device limits. */
	struct PhysicalDeviceLimits
	{
	public:
		uint32 MaxImageDimension1D;
		uint32 MaxImageDimension2D;
		uint32 MaxImageDimension3D;
		uint32 MaxImageDimensionCube;
		uint32 MaxImageArrayLayers;
		uint32 MaxTexelBufferElements;
		uint32 MaxUniformBufferRange;
		uint32 MaxStorageBufferRange;
		uint32 MaxPushConstantsSize;
		uint32 MaxMemoryAllocationCount;
		uint32 MaxSamplerAllocationCount;
		DeviceSize BufferImageGranularity;
		DeviceSize SparseAddressSpaceSize;
		uint32 MaxBoundDescriptorSets;
		uint32 MaxPerStageDescriptorSamplers;
		uint32 MaxPerStageDescriptorUniformBuffers;
		uint32 MaxPerStageDescriptorStorageBuffers;
		uint32 MaxPerStageDescriptorSampledImages;
		uint32 MaxPerStageDescriptorStorageImages;
		uint32 MaxPerStageDescriptorInputAttachments;
		uint32 MaxPerStageResources;
		uint32 MaxDescriptorSetSamplers;
		uint32 MaxDescriptorSetUniformBuffers;
		uint32 MaxDescriptorSetUniformBuffersDynamic;
		uint32 MaxDescriptorSetStorageBuffers;
		uint32 MaxDescriptorSetStorageBuffersDynamic;
		uint32 MaxDescriptorSetSampledImages;
		uint32 MaxDescriptorSetStorageImages;
		uint32 MaxDescriptorSetInputAttachments;
		uint32 MaxVertexInputAttributes;
		uint32 MaxVertexInputBindings;
		uint32 MaxVertexInputAttributeOffset;
		uint32 MaxVertexInputBindingStride;
		uint32 MaxVertexOutputComponents;
		uint32 MaxTessellationGenerationLevel;
		uint32 MaxTessellationPatchSize;
		uint32 MaxTessellationControlPerVertexInputComponents;
		uint32 MaxTessellationControlPerVertexOutputComponents;
		uint32 MaxTessellationControlPerPatchOutputComponents;
		uint32 MaxTessellationControlTotalOutputComponents;
		uint32 MaxTessellationEvaluationInputComponents;
		uint32 MaxTessellationEvaluationOutputComponents;
		uint32 MaxGeometryShaderInvocations;
		uint32 MaxGeometryInputComponents;
		uint32 MaxGeometryOutputComponents;
		uint32 MaxGeometryOutputVertices;
		uint32 MaxGeometryTotalOutputComponents;
		uint32 MaxFragmentInputComponents;
		uint32 MaxFragmentOutputAttachments;
		uint32 MaxFragmentDualSrcAttachments;
		uint32 MaxFragmentCombinedOutputResources;
		uint32 MaxComputeSharedMemorySize;
		uint32 MaxComputeWorkGroupCount[3];
		uint32 MaxComputeWorkGroupInvocations;
		uint32 MaxComputeWorkGroupSize[3];
		uint32 SubPixelPrecisionBits;
		uint32 SubTexelPrecisionBits;
		uint32 MipmapPrecisionBits;
		uint32 MaxDrawIndexedIndexValue;
		uint32 MaxDrawIndirectCount;
		float MaxSamplerLodBias;
		float MaxSamplerAnisotropy;
		uint32 MaxViewports;
		uint32 MaxViewportDimensions[2];
		float ViewportBoundsRange[2];
		uint32 ViewportSubPixelBits;
		size_t MinMemoryMapAlignment;
		DeviceSize MinTexelBufferOffsetAlignment;
		DeviceSize MinUniformBufferOffsetAlignment;
		DeviceSize MinStorageBufferOffsetAlignment;
		int32_t MinTexelOffset;
		uint32 MaxTexelOffset;
		int32_t MinTexelGatherOffset;
		uint32 MaxTexelGatherOffset;
		float MinInterpolationOffset;
		float MaxInterpolationOffset;
		uint32 SubPixelInterpolationOffsetBits;
		uint32 MaxFramebufferWidth;
		uint32 MaxFramebufferHeight;
		uint32 MaxFramebufferLayers;
		SampleCountFlag FramebufferColorSampleCounts;
		SampleCountFlag FramebufferDepthSampleCounts;
		SampleCountFlag FramebufferStencilSampleCounts;
		SampleCountFlag FramebufferNoAttachmentsSampleCounts;
		uint32 MaxColorAttachments;
		SampleCountFlag SampledImageColorSampleCounts;
		SampleCountFlag SampledImageIntegerSampleCounts;
		SampleCountFlag SampledImageDepthSampleCounts;
		SampleCountFlag SampledImageStencilSampleCounts;
		SampleCountFlag StorageImageSampleCounts;
		uint32 MaxSampleMaskWords;
		Bool32 TimestampComputeAndGraphics;
		float TimestampPeriod;
		uint32 MaxClipDistances;
		uint32 MaxCullDistances;
		uint32 MaxCombinedClipAndCullDistances;
		uint32 DiscreteQueuePriorities;
		float PointSizeRange[2];
		float LineWidthRange[2];
		float PointSizeGranularity;
		float LineWidthGranularity;
		Bool32 StrictLines;
		Bool32 StandardSampleLocations;
		DeviceSize OptimalBufferCopyOffsetAlignment;
		DeviceSize OptimalBufferCopyRowPitchAlignment;
		DeviceSize NonCoherentAtomSize;
	};

	/* Defines the physical device sparse memory properties. */
	struct PhysicalDeviceSparseProperties
	{
	public:
		/* True if the physical device will access all single-sample 2D sprase resources using the standard sparse image block shapes. */
		Bool32 ResidencyStandard2DBlockShape;
		/* True if the physical device will access all multisample 2D sprase resources using the standard sparse image block shapes. */
		Bool32 ResidencyStandard2DMultisampleBlockShape;
		/* True if the physical device will access all 3D sprase resources using the standard sparse image block shapes. */
		Bool32 ResidencyStandard3DBlockShape;
		/* True if images with mip level dimensions that are not integer multiples of the corresponding dimensions of the sparse image block may be places in the mip tail. */
		Bool32 ResidencyAlignedMipSize;
		/* Specifies whether the physical device can consistently access non-resident regions of a resource. */
		Bool32 ResidencyNonResidentStrict;

		/* Initializes an empty instance of a physical device sparse accessor properties. */
		PhysicalDeviceSparseProperties(void)
			: ResidencyStandard2DBlockShape(false), ResidencyStandard2DMultisampleBlockShape(false),
			ResidencyStandard3DBlockShape(false), ResidencyAlignedMipSize(false), ResidencyNonResidentStrict(false)
		{}
	};

	/* Defines the physical device properties. */
	struct PhysicalDeviceProperties
	{
	public:
		/* The version of Vulkan supported by the device. */
		uint32 ApiVersion;
		/* The vendor-specified version of the driver. */
		uint32 DriverVersion;
		/* A unique identifier for the vendor. */
		uint32 VendorID;
		/* A unique indentifier for the physical device. */
		uint32 DeviceID;
		/* Specifies teh type of the device. */
		PhysicalDeviceType DeviceType;
		/* A null-terminated UTF-8 string containing the name of the device. */
		char DeviceName[MaxPhysicalDeviceNameSize];
		/* Specifies the universally unique indentifier for the device. */
		uint8_t PipelineCacheUUID[UUIDSize];
		/* Specifies the device-specific limits of the physical device. */
		PhysicalDeviceLimits Limits;
		/* Specifies various sparse related properties of the physical device. */
		PhysicalDeviceSparseProperties SparseProperties;

		/* Initializes an empty instance of a physical device properties object. */
		PhysicalDeviceProperties(void)
			: ApiVersion(makeVersion(0, 0, 0)), DriverVersion(0), VendorID(0), DeviceID(0),
			DeviceType(PhysicalDeviceType::Other), DeviceName(u8"Invalid"), Limits(), SparseProperties()
		{}
	};

	/* Defines the properties of an extension. */
	struct ExtensionProperties
	{
	public:
		/* A null-terminated UTF-8 string specifying the name of the extension. */
		char ExtensionName[MaxExtensionNameSize];
		/* Specifies the version of the extension. */
		uint32 SpecVersion;

		/* Initializes an empty instance of an extension properties object. */
		ExtensionProperties(void)
			: ExtensionName("Invalid"), SpecVersion(makeVersion(0, 0, 0))
		{}
	};

	/* Defines the properties of a layer. */
	struct LayerProperties
	{
	public:
		/* A null-terminated UTF-8 string specifying the name of the layer. */
		char LayerName[MaxExtensionNameSize];
		/* Specifies the Vulkan version the layer was written to. */
		uint32 SpecVersion;
		/* Specifies the version of the layer. */
		uint32 ImplementationVersion;
		/* A null-terminated UTF-8 string providing additional details. */
		char Description[MaxDescriptionSize];

		/* Initializes an empty instance of a layer properties object. */
		LayerProperties(void)
			: LayerName(u8"Invalid"), SpecVersion(makeVersion(0, 0, 0)),
			ImplementationVersion(makeVersion(0, 0, 0)), Description(u8"")
		{}
	};

	/* Defines information about a queue family. */
	struct QueueFamilyProperties
	{
	public:
		/* Specifies the capabilities of the queues in this queue family. */
		QueueFlag Flags;
		/* Specifies the amount of queus in the queue family. */
		uint32 QueueCount;
		/* Specifies the meaningful bits in the timestamps, valid range: [36, 64] or zero. */
		uint32 TimestampValidBits;
		/* Specified the minimum granularity supported for image transfer operations. */
		Extent3D MinImageTransferGranularity;

		/* Initializes an empty instance of a queue family properties object. */
		QueueFamilyProperties(void)
			: Flags(QueueFlag::None), QueueCount(0), TimestampValidBits(0), MinImageTransferGranularity()
		{}
	};

	/* Defines information for creating new device queues. */
	struct DeviceQueueCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies behaviour of the queue. */
		DeviceQueueCreateFlag Flags;
		/* Indicates the index of the queu family to create on this device. */
		uint32 QueueFamilyIndex;
		/* Specifies the amount of queues to create in the queue family. */
		uint32 Count;
		/* Specified (normalized) priorities of work that will be submitted to each created queue. */
		const float *QueuePriorities;

		/* Initializes an empty instance of a queue create information object. */
		DeviceQueueCreateInfo(void)
			: DeviceQueueCreateInfo(0, 0, nullptr)
		{}

		/* Initializes a new instance of a queue create information object. */
		DeviceQueueCreateInfo(_In_ uint32 familyIndex, _In_ uint32 count, _In_ const float *priorities, _In_opt_ DeviceQueueCreateFlag flags = DeviceQueueCreateFlag::None)
			: Type (StructureType::DeviceQueueCreateInfo), Next(nullptr),
			Flags(flags), QueueFamilyIndex(familyIndex), Count(count), QueuePriorities(priorities)
		{}
	};

	/* Defines information for creating new logical devices. */
	struct DeviceCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Must be zero. */
		Flags Flags;
		/* Specifies the amount to queue create information objects passed. */
		uint32 QueueCreateInfoCount;
		/* Specifies the information describing the queues that are requested to be created along with the logical device. */
		const DeviceQueueCreateInfo *QueueCreateInfos;
		/* Deprecated. */
		uint32 EnabledLayerCount;
		/* Deprecated. */
		const char *const *EnabledLayerNames;
		/* Specifies the amount of device extensions to enable. */
		uint32 EnabledExtensionCount;
		/* Specifies the extensions to enable for the created logical device. */
		const char *const *EnabledExtensionNames;
		/* Specifies the features to enable for the logical device or nullptr. */
		const PhysicalDeviceFeatures *EnabledFeatures;

		/* Creates an empty instance of the device creating information object. */
		DeviceCreateInfo(void)
			: DeviceCreateInfo(0, nullptr)
		{}

		/* Creates a new instance of the device create information object. */
		DeviceCreateInfo(_In_ uint32 queueCreateInfoCount, _In_ const DeviceQueueCreateInfo *queueCreateInfos, 
			_In_opt_ uint32 enabledExtensionCount = 0, _In_opt_ const char *const *enabledExtensionNames = nullptr, 
			_In_opt_ const PhysicalDeviceFeatures *enabledFeatures = nullptr)
			: Type (StructureType::DeviceCreatInfo), Next(nullptr), Flags(0),
			EnabledLayerCount(0), EnabledLayerNames(nullptr),
			QueueCreateInfoCount(queueCreateInfoCount), QueueCreateInfos(queueCreateInfos), 
			EnabledExtensionCount(enabledExtensionCount), EnabledExtensionNames(enabledExtensionNames), EnabledFeatures(enabledFeatures)
		{}
	};

	/* Defines the capabilities of a surface. */
	struct SurfaceCapabilities
	{
	public:
		/* The minimum number of images the specified device supports for a swapchain. */
		uint32 MinImageCount;
		/* The maximum number of images the specified device supports for a swapchain. */
		uint32 MaxImageCount;
		/* The current width and height of the surface.  */
		Extent2D CurrentExtent;
		/* The smallest valid width and height of the surface. */
		Extent2D MinImageExtent;
		/* The largest valid width and height of the surface. */
		Extent2D MaxImageExtent;
		/* The maximum number of layers a swapchain image can have. */
		uint32 MaxImageArrayLayers;
		/* Specifies the supported surface transforms for the surface. */
		SurfaceTransformFlag SupportedTransforms;
		/* Specifies the current surface transform. */
		SurfaceTransformFlag CurrentTransform;
		/* Specifies the supported alpha composition models. */
		CompositeAlphaFlag SupportedCompositeAlpha;
		/* Specifies the way applications can use the presentable images of a swapchain created for the surface. */
		ImageUsageFlag SupportedUsages;

		/* Initializes an empty instance of the surface capabilities object. */
		SurfaceCapabilities(void)
			: MinImageCount(0), MaxImageCount(0), CurrentExtent(), MinImageExtent(), MaxImageExtent(),
			MaxImageArrayLayers(0), SupportedTransforms(SurfaceTransformFlag::Identity), CurrentTransform(SurfaceTransformFlag::Identity),
			SupportedCompositeAlpha(CompositeAlphaFlag::Inherit), SupportedUsages(ImageUsageFlag::None)
		{}

		/* Checks whether the current extent is determined by the extent of the swapchain targeting the surface. */
		_Check_return_ inline bool IsExtentAuto(void) const
		{
			return CurrentExtent.Width == 0xFFFFFFFF && CurrentExtent.Height == 0xFFFFFFFF;
		}
	};

	/* Defines the surface pixel format. */
	struct SurfaceFormat
	{
	public:
		/* The color format compatible with the surface. */
		Format Format;
		/* The color space compatible with the surface. */
		ColorSpace ColorSpace;

		/* Initializes an empty instance of a surface pixel format object. */
		SurfaceFormat(void)
			: Format(Format::Undefined), ColorSpace(ColorSpace::SRGB)
		{}

		/* Initializes a new instance of a surface pixel format object. */
		SurfaceFormat(_In_ Pu::Format format, _In_ Pu::ColorSpace colorSpace)
			: Format(format), ColorSpace(colorSpace)
		{}

		/* Gets the string representation of the surface format. */
		_Check_return_ inline operator string(void) const
		{
			string result = to_string(ColorSpace);
			result += " (";
			result += to_string(Format);
			result += ')';
			return result;
		}

		/* Checks whether the two surface formats are the same. */
		_Check_return_ inline bool operator ==(_In_ const SurfaceFormat &other) const
		{
			return Format == other.Format && ColorSpace == other.ColorSpace;
		}

		/* Checks whether the two surface formats differ. */
		_Check_return_ inline bool operator !=(_In_ const SurfaceFormat &other) const
		{
			return Format != other.Format || ColorSpace != other.ColorSpace;
		}
	};

	/* Defines the information required to create a swapchain. */
	struct SwapchainCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* The surface that the swapchain will present to. */
		SurfaceHndl Surface;
		/* Specifies the minimum number of presentable images that the application needs. */
		uint32 MinImageCount;
		/* Specifies the pixel format for the presentable images. */
		Format ImageFormat;
		/* Specifies the color space of the presentable images. */
		ColorSpace ImageColorSpace;
		/* the size (in pixels) of the swapchain images. */
		Extent2D ImageExtent;
		/* The number of views in a multiview/stereo surface. */
		uint32 ImageArrayLayers;
		/* Specifies how the application will use the swapchain's presentable images. */
		ImageUsageFlag ImageUsage;
		/* Specifies the sharing mode for the presentable images. */
		SharingMode ImageSharingMode;
		/* The number of queue families having access to the images of the swapchain. */
		uint32 QueueFamilyIndexCount;
		/* The queue families having access to the images of the swapchain. */
		const uint32 *QueueFamilyIndeces;
		/* Specifies the transform applied to the presentable images when presenting. */
		SurfaceTransformFlag Transform;
		/* Specifies the alpha composition mode of the images. */
		CompositeAlphaFlag CompositeAlpha;
		/* Specifies the presentation mode the swapchain will use. */
		PresentMode PresentMode;
		/* Specifies whether the Vulkan implementation is allowed to discard rendering operataions that affect regions of the surface which are not visible. */
		Bool32 Clipped;
		/* Specifies an optional swapchain that will be replaced with the new one. */
		SwapchainHndl OldSwapChain;

		/* Initializes an empty instance of a swapchain creation info object. */
		SwapchainCreateInfo(void)
			: SwapchainCreateInfo(nullptr, Extent2D())
		{}

		/* Initializes a new instance of the swapchain creation info object. */
		SwapchainCreateInfo(_In_ SurfaceHndl surface, _In_ Extent2D size)
			: Type(StructureType::SwapChainCreateInfoKhr), Next(nullptr), Flags(0),
			Surface(surface), MinImageCount(3), ImageFormat(Format::Undefined),
			ImageColorSpace(ColorSpace::SRGB), ImageExtent(size), ImageArrayLayers(1),
			ImageUsage(ImageUsageFlag::None), ImageSharingMode(SharingMode::Exclusive),
			QueueFamilyIndexCount(0), QueueFamilyIndeces(nullptr), Transform(SurfaceTransformFlag::Identity),
			CompositeAlpha(CompositeAlphaFlag::Opaque), PresentMode(PresentMode::MailBox),
			Clipped(true), OldSwapChain(nullptr)
		{}
	};

	/* Defines information for presenting images. */
	struct PresentInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the number of semaphores to wait for before issuing the present request. */
		uint32 WaitSemaphoreCount;
		/* Specified the semaphores the wait for before issuing the present request. */
		const SemaphoreHndl *WaitSemaphores;
		/* Specifies the number of swapchains being presented to by this command. */
		uint32 SwapchainCount;
		/* Specifies the unique swapchains being presented to.  */
		const SwapchainHndl *Swapchains;
		/* Specifies the images to present or the corresponding swapchain. */
		const uint32 *ImageIndeces;
		/* The result of the operation per specified spawchain. */
		VkApiResult *result;

		/* Initializes an empty instance of the present info object. */
		PresentInfo(void)
			: PresentInfo(0, nullptr, nullptr)
		{}

		/* Initializes a new instance of the present info object. */
		PresentInfo(_In_ uint32 swapchainCount, _In_ const SwapchainHndl *swapchain, _In_ const uint32 *imageIndeces)
			: Type(StructureType::PresentInfoKhr), Next(nullptr),
			WaitSemaphoreCount(0), WaitSemaphores(nullptr),
			SwapchainCount(swapchainCount), Swapchains(swapchain),
			ImageIndeces(imageIndeces), result(nullptr)
		{}
	};

	/* Defines the information required to create a semaphore. */
	struct SemaphoreCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;

		SemaphoreCreateInfo(void)
			: Type(StructureType::SemaphoreCreateInfo), Next(nullptr), Flags(0)
		{}
	};

	/* Defines the information required to perform a queue submit operation. */
	struct SubmitInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the number of semaphores upon which to wait before executing the command buffers. */
		uint32 WaitSemaphoreCount;
		/* Specifies the semaphores upon which to wait before executing the command buffers. */
		const SemaphoreHndl *WaitSemaphores;
		/* Specifies pipeline stages at which each corresponding semaphore wait will occur. */
		const PipelineStageFlag *WaitDstStageMask;
		/* Specifies the amount of command buffers to execute in the batch. */
		uint32 CommandBufferCount;
		/* Specified the command buffers to execute in the batch. */
		const CommandBufferHndl *CommandBuffers;
		/* Specifies the number of semaphores to be signaled once the commands have completed execution. */
		uint32 SignalSemaphoreCount;
		/* Specifies the semaphores to be signaled once the command have completed executing. */
		const SemaphoreHndl *SignalSemaphores;

		/* initializes an empty instance of the queue submit info object. */
		SubmitInfo(void)
			: Type(StructureType::SubmitInfo), Next(nullptr), WaitSemaphoreCount(0),
			WaitSemaphores(nullptr), WaitDstStageMask(nullptr), CommandBufferCount(0),
			CommandBuffers(nullptr), SignalSemaphoreCount(0), SignalSemaphores(nullptr)
		{}
	};

	/* Defines the information required to create a new command pool. */
	struct CommandPoolCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Indicates usage behaviour for the pool and command buffers allocated from it. */
		CommandPoolCreateFlag Flags;
		/* Specifies the designated queue family. */
		uint32 QueueFamilyIndex;

		/* Initializes an empty instance of the command pool create info object. */
		CommandPoolCreateInfo(void)
			: CommandPoolCreateInfo(0, CommandPoolCreateFlag::None)
		{}

		/* Initializes a new instance of the command pool create info object. */
		CommandPoolCreateInfo(_In_ uint32 queueFamilyIndex, _In_ CommandPoolCreateFlag flags)
			: Type(StructureType::CommandPoolCreateInfo), Next(nullptr),
			Flags(flags), QueueFamilyIndex(queueFamilyIndex)
		{}
	};

	/* Defines the information required to allocate a new command buffer. */
	struct CommandBufferAllocateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* The command pool from which the command buffers are allocated. */
		CommandPoolHndl CommandPool;
		/* Specifies the level of the command buffers. */
		CommandBufferLevel Level;
		/* Specifies the amount of command buffers to allocate from the pool. */
		uint32 CommandBufferCount;

		/* Initializes an empty instance of the command buffer allocation info object. */
		CommandBufferAllocateInfo(void)
			: CommandBufferAllocateInfo(nullptr, 0)
		{}

		/* Initializes a new instance of the command buffer allocation info object. */
		CommandBufferAllocateInfo(_In_ CommandPoolHndl commandPool, _In_ uint32 count)
			: Type(StructureType::CommandBufferAllocateInfo), Next(nullptr),
			CommandPool(commandPool), Level(CommandBufferLevel::Primary), CommandBufferCount(count)
		{}
	};

	/* Defines the inheritance information for secondary conmmand buffers. */
	struct CommandBufferInheritanceInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies which renderpass the command buffer will be compatible with. */
		RenderPassHndl RenderPass;
		/* Specifies the index of the subpass within the render pass that the command buffer will be executed within. */
		uint32 Subpass;
		/* Specifies the framebuffer that the command buffer will render to. */
		FramebufferHndl FrameBuffer;
		/* Specifies whether the command buffer can be executed while an occlusion query is active in the primary command buffer. */
		Bool32 OcclusionQueryEnable;
		/* Specifies the flags that can be used by an active occlusion query in the primary command buffer. */
		QueryControlFlag QueryFlags;
		/* Specifies the statistics that can be counted by an active query. */
		QueryPipelineStatisticFlag PipelineStatistics;

		/* Initializes an empty instance of a secondary command buffer inheritance info object. */
		CommandBufferInheritanceInfo(void)
			: CommandBufferInheritanceInfo(nullptr)
		{}

		/* Initializes a new instance of a secondaly command buffer inheritance info object. */
		CommandBufferInheritanceInfo(_In_ RenderPassHndl renderPass)
			: Type(StructureType::CommandBufferInheritanceInfo), Next(nullptr), RenderPass(renderPass),
			Subpass(0), FrameBuffer(nullptr), OcclusionQueryEnable(false), 
			QueryFlags(QueryControlFlag::None), PipelineStatistics(QueryPipelineStatisticFlag::None)
		{}
	};

	/* Defines a command buffer begin operation. */
	struct CommandBufferBeginInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the usage of the command buffer. */
		CommandBufferUsageFlag Flags;
		/* Specifies extra information for secondary command buffers. */
		const CommandBufferInheritanceInfo *InheritanceInfo;

		/* Initializes a new instance of the command buffer begin info object. */
		CommandBufferBeginInfo(void)
			: Type(StructureType::CommandBufferBeginInfo), Next(nullptr),
			Flags(CommandBufferUsageFlag::None), InheritanceInfo(nullptr)
		{}
	};

	/*  Defines an image subresource range. */
	struct ImageSubresourceRange
	{
	public:
		/* Specifies which aspect(s) of the image are included in the view. */
		ImageAspectFlag AspectMask;
		/* Specifies the first mipmap level accessible to the view. */
		uint32 BaseMipLevel;
		/* Specifies the number of mipmap levels accessible to the view. */
		uint32 LevelCount;
		/* Specifies the first array layer accessible to the view. */
		uint32 BaseArraylayer;
		/* Specifies the number of array layers accessible to the view. */
		uint32 LayerCount;

		/* Initializes an empty instance of the image subresrouce range object. */
		ImageSubresourceRange(void)
			: ImageSubresourceRange(ImageAspectFlag::Color)
		{}

		/* Initializes a new instace of the image subresource range object. */
		ImageSubresourceRange(_In_ ImageAspectFlag aspect)
			: AspectMask(aspect), BaseMipLevel(0),
			LevelCount(1), BaseArraylayer(0), LayerCount(1)
		{}
	};

	/* Defines the required information of an image memory barrier. */
	struct ImageMemoryBarrier
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the access mask for the source. */
		AccessFlag SrcAccessMask;
		/* Specifies the access mask for the destination. */
		AccessFlag DstAccessMask;
		/* Specifies the old image layout. */
		ImageLayout OldLayout;
		/* Specifies the new image layout. */
		ImageLayout NewLayout;
		/* Specifies the queue family index for the source. */
		uint32 SrcQueueFamilyIndex;
		/* Specifies the queue family index for the destination. */
		uint32 DstQueueFamilyIndex;
		/* Specifies the image affected by this barrier. */
		ImageHndl Image;
		/* Specifies the sub range affected by this barrier. */
		ImageSubresourceRange SubresourceRange;

		/* Initializes an empty instance of the image memory barrier object. */
		ImageMemoryBarrier(void)
			: ImageMemoryBarrier(nullptr, 0)
		{}

		/* Initializes a new instance of the image memory barrier object. */
		ImageMemoryBarrier(_In_ ImageHndl image, _In_ uint32 queueFamilyIdx)
			: Type(StructureType::ImageMemoryBarrier), Next(nullptr),
			SrcAccessMask(AccessFlag::None), DstAccessMask(AccessFlag::None),
			OldLayout(ImageLayout::Undefined), NewLayout(ImageLayout::Undefined),
			SrcQueueFamilyIndex(queueFamilyIdx), DstQueueFamilyIndex(queueFamilyIdx), Image(image), SubresourceRange()
		{}
	};

	/* Defines a clear color value. */
	union ClearColorValue
	{
		/* Used when the format of the image is one of the numeric formats. */
		float float32[4];
		/* Used when the format of the image is SINT. */
		int32 int32[4];
		/* Used when the format of the image is UINT. */
		uint32 uint32[4];
	};

	/* Defines a global memory barrier. */
	struct MemoryBarrier
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the source access mask. */
		AccessFlag SrcAccessMask;
		/* Specifies the destination access mask. */
		AccessFlag DstAccessMask;

		/* Initializes an empty instance of a global memory barrier. */
		MemoryBarrier()
			: MemoryBarrier(AccessFlag::None, AccessFlag::None) 
		{}

		/* Initializes a new instance of a global memory barrier. */
		MemoryBarrier(_In_ AccessFlag src, _In_ AccessFlag dst)
			: Type(StructureType::MemoryBarrier), Next(nullptr),
			SrcAccessMask(src), DstAccessMask(dst)
		{}
	};

	/* Defines a buffer memory barrier. */
	struct BufferMemoryBarrier
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the access mask for the source. */
		AccessFlag SrcAccessMask;
		/* Specifies the access mask for the destination. */
		AccessFlag DstAccessMask;
		/* Specifies the queue family index for the source. */
		uint32 SrcQueueFamilyIndex;
		/* Specifies the queue family index for the destination. */
		uint32 DstQueueFamilyIndex;
		/* Specifies the buffer whose memory is affected by the barrier. */
		BufferHndl Buffer;
		/* Specifies the offset (in bytes) from where to start. */
		DeviceSize Offset;
		/* Specifies the size (in bytes) of the affected area. */
		DeviceSize Size;

		/* Initializes an empty instance of the buffer memory barrier object. */
		BufferMemoryBarrier(void)
			: BufferMemoryBarrier(nullptr, 0, 0)
		{}

		/* Initializes a new instance of the buffer memory barrier object. */
		BufferMemoryBarrier(_In_ BufferHndl buffer, _In_opt_ DeviceSize offset = 0, _In_opt_ DeviceSize size = WholeSize)
			: Type(StructureType::BufferMemoryBarrier), Next(nullptr),
			SrcAccessMask(AccessFlag::None), DstAccessMask(AccessFlag::None),
			SrcQueueFamilyIndex(QueueFamilyIgnored), DstQueueFamilyIndex(QueueFamilyIgnored), Buffer(buffer),
			Offset(offset), Size(size)
		{}
	};

	/* Defines information that describes an attachment. */
	struct AttachmentDescription
	{
	public:
		/* Specifies additional properties of the attachment. */
		AttachmentDescriptionFlag Flags;
		/* Specifies the format of the image view that will be used for the attachment. */
		Format Format;
		/* Specified the amount of samples of the image. */
		SampleCountFlag Samples;
		/* Specifies how the color and depth components should be treated at the beginning of the subpass. */
		AttachmentLoadOp LoadOp;
		/* Specifies what should be done with the color and depth components after the subpass. */
		AttachmentStoreOp StoreOp;
		/* Specifies how the stencil component (if present) should be treated at the beginning of the subpass. */
		AttachmentLoadOp StencilLoadOp;
		/* Specifies what should be done with stencil comoponent (if present) after the subpass. */
		AttachmentStoreOp StencilStoreOp;
		/* Specifies the initial layout of the image subresource. */
		ImageLayout InitialLayout;
		/* Specifies the layout of the image subresource after the subpass. */
		ImageLayout FinalLayout;

		/* Initializes an empty instance of the attachment description object. */
		AttachmentDescription(void)
			: AttachmentDescription(Format::Undefined, ImageLayout::Undefined, ImageLayout::Undefined)
		{}

		/* Initializes a new instance of the attachment description object. */
		AttachmentDescription(_In_ Pu::Format format, _In_ ImageLayout initialLayout, _In_ ImageLayout finalLayout)
			: Flags(AttachmentDescriptionFlag::None), Format(format), Samples(SampleCountFlag::Pixel1Bit),
			LoadOp(AttachmentLoadOp::Clear), StoreOp(AttachmentStoreOp::Store),
			StencilLoadOp(AttachmentLoadOp::DontCare), StencilStoreOp(AttachmentStoreOp::DontCare),
			InitialLayout(initialLayout), FinalLayout(finalLayout)
		{}
	};

	/* Defines a refrence to a specified attachment. */
	struct AttachmentReference
	{
	public:
		/* Defines an attachment pointer that indicates that the attachment reference is not in use. */
		static constexpr const AttachmentReference *Unused = reinterpret_cast<const AttachmentReference*>(static_cast<uint64>(~0U));

		/* The index of the attachment of the render pass. */
		uint32 Attachment;
		/* Specifies the layout the attachment uses during the subpass. */
		ImageLayout Layout;

		/* Initializes an empty instance of a attachment refrence. */
		AttachmentReference(void)
			: AttachmentReference(~0U, ImageLayout::Undefined)
		{}

		/* Initializes a new instance of a attachment reference. */
		AttachmentReference(_In_ uint32 attachmentIndex, _In_ ImageLayout layout)
			: Attachment(attachmentIndex), Layout(layout)
		{}
	};

	/* Defines information describing a subpass. */
	struct SubpassDescription
	{
	public:
		/* Specifies the usage of the subpass. */
		SubpassDescriptionFlag Flags;
		/* Specifies whether this is a compute or graphics subpass. */
		PipelineBindPoint BindPoint;
		/* Specifies the amount of input attachments. */
		uint32 InputAttachmentCount;
		/* Specifies which of the render pass's attachments can be read in the input shader stage. */
		const AttachmentReference *InputAttachments;
		/* Specifies the amount of color attachments. */
		uint32 ColorAttachmentCount;
		/* Specifies which of the render pass's attachments will be used as color attachments in the subpass. */
		const AttachmentReference *ColorAttachments;
		/* Specifies which of the render pass's attachments are resolved at the end of the subpass. */
		const AttachmentReference *ResolveAttachments;
		/* Specifies which attachment will be used for depth/stencil data. */
		const AttachmentReference *DepthStencilAttachment;
		/* Specifies the amount of preserved attachments. */
		uint32 PreserveAttachmentCount;
		/* Specifies which attachments are not used by a subpass, but whose contents must be preserved throughout the subpass. */
		const AttachmentReference *PreserveAttachments;

		/* Initializes an empty instance of a subpass description object. */
		SubpassDescription(void)
			: Flags(SubpassDescriptionFlag::None), BindPoint(PipelineBindPoint::Graphics),
			InputAttachmentCount(0), InputAttachments(nullptr), ColorAttachmentCount(0),
			ColorAttachments(nullptr), ResolveAttachments(nullptr), DepthStencilAttachment(nullptr),
			PreserveAttachmentCount(0), PreserveAttachments(nullptr)
		{}

		/* Initializes a new instance of a subpass description object. */
		SubpassDescription(_In_ const AttachmentReference &colorAttachment)
			: Flags(SubpassDescriptionFlag::None), BindPoint(PipelineBindPoint::Graphics),
			InputAttachmentCount(0), InputAttachments(nullptr),
			ColorAttachmentCount(1), ColorAttachments(&colorAttachment),
			ResolveAttachments(nullptr), DepthStencilAttachment(nullptr),
			PreserveAttachmentCount(0), PreserveAttachments(nullptr)
		{}

		/* Initializes a new instance of a subpass description object. */
		SubpassDescription(_In_ const vector<AttachmentReference> &colorAttachments,
			_In_ const vector<AttachmentReference> &inputAttachments, _In_ const vector<AttachmentReference> &resolveAttachments)
			: Flags(SubpassDescriptionFlag::None), BindPoint(PipelineBindPoint::Graphics), DepthStencilAttachment(nullptr),
			InputAttachmentCount(static_cast<uint32>(inputAttachments.size())), InputAttachments(inputAttachments.data()), 
			ColorAttachmentCount(static_cast<uint32>(colorAttachments.size())), 
			ColorAttachments(colorAttachments.data()), ResolveAttachments(resolveAttachments.data()),
			PreserveAttachmentCount(0), PreserveAttachments(nullptr)
		{}
	};

	/* Defines a subpass dependency. */
	struct SubpassDependency
	{
	public:
		/* Specifies the index of the first subpass in the dependency. */
		uint32 SrcSubpass;
		/* Specifies the index of this subpass in the dependency. */
		uint32 DstSubpass;
		/* Specifies the source stage mask. */
		PipelineStageFlag SrcStageMask;
		/* Specifies the destination stage mask. */
		PipelineStageFlag DstStageMask;
		/* Specifies the source access. */
		AccessFlag SrcAccessMask;
		/* Specifies the destination access. */
		AccessFlag DstAccessMask;
		/* Specifies optional parameters for the subpass dependency.  */
		DependencyFlag DependencyFlags;

		/* Initializes an empty instance of a subpass dependency. */
		SubpassDependency(void)
			: SubpassDependency(0, 0)
		{}

		/* Initializes a new instance of a subpass dependency. */
		SubpassDependency(_In_ uint32 source, _In_ uint32 destination)
			: SrcSubpass(source), DstSubpass(destination), 
			SrcStageMask(PipelineStageFlag::AllCommands), DstStageMask(PipelineStageFlag::AllCommands),
			SrcAccessMask(AccessFlag::None), DstAccessMask(AccessFlag::None), DependencyFlags(DependencyFlag::None)
		{}
	};

	/* Defines the information required to create a render pass. */
	struct RenderPassCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the amount of attachments used by this render pass. */
		uint32 AttachmentCount;
		/* Specifies the attachments used by this render pass. */
		const AttachmentDescription *Attachments;
		/* Specifies the amount of subpasses to create for this render pass. */
		uint32 SubpassCount;
		/* Specifies the subpasses for this render pass. */
		const SubpassDescription *Subpasses;
		/* Specifies the amount of dependencies between pairs of subpasses. */
		uint32 DependencyCount;
		/* Specifies the dependenciers between pairs of subpasses. */
		const SubpassDependency *Dependencies;

		/* Initializes an empty instance of the render pass create info object. */
		RenderPassCreateInfo(void)
			: Type(StructureType::RenderPassCreateInfo), Next(nullptr), Flags(0),
			AttachmentCount(0), Attachments(nullptr), SubpassCount(0), Subpasses(nullptr),
			DependencyCount(0), Dependencies(nullptr)
		{}

		/* Initializes a new instance of a renderpass create info object. */
		RenderPassCreateInfo(_In_ const AttachmentDescription &attachment, _In_ const SubpassDescription &subpass, _In_ const SubpassDependency &depencency)
			: Type(StructureType::RenderPassCreateInfo), Next(nullptr), Flags(0),
			AttachmentCount(1), Attachments(&attachment),
			SubpassCount(1), Subpasses(&subpass),
			DependencyCount(1), Dependencies(&depencency)
		{}

		/* Initializes a new instance of a renderpass create info object. */
		RenderPassCreateInfo(_In_ const vector<AttachmentDescription> &attachments, _In_ const SubpassDescription &subpass, _In_ const vector<SubpassDependency> &dependencies)
			: Type(StructureType::RenderPassCreateInfo), Next(nullptr), Flags(0),
			AttachmentCount(static_cast<uint32>(attachments.size())), Attachments(attachments.data()),
			SubpassCount(1), Subpasses(&subpass), 
			DependencyCount(static_cast<uint32>(dependencies.size())), Dependencies(dependencies.data())
		{}

		/* Initializes a new instance of the renderpass create info object. */
		RenderPassCreateInfo(_In_ const vector<AttachmentDescription> &attachments, _In_ const vector<SubpassDescription> &subpasses, _In_ const vector<SubpassDependency> &dependencies)
			: Type(StructureType::RenderPassCreateInfo), Next(nullptr), Flags(0),
			AttachmentCount(static_cast<uint32>(attachments.size())), Attachments(attachments.data()),
			SubpassCount(static_cast<uint32>(subpasses.size())), Subpasses(subpasses.data()),
			DependencyCount(static_cast<uint32>(dependencies.size())), Dependencies(dependencies.data())
		{}
	};

	/* Defines the information required to create a new shader module. */
	struct ShaderModuleCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the size (in bytes) of the code. */
		size_t CodeSize;
		/* Specifies the SPIR-V code used to create the shader module. */
		const uint32 *Code;

		/* Creates an empty instance of the shader module create info object. */
		ShaderModuleCreateInfo(void)
			: ShaderModuleCreateInfo(0, nullptr)
		{}

		/* Initializes a new instance of the shader module create info object. */
		ShaderModuleCreateInfo(_In_ size_t size, _In_ const void *code)
			: Type(StructureType::ShaderModuleCreateInfo), Next(nullptr), Flags(0),
			CodeSize(size), Code(reinterpret_cast<const uint32*>(code))
		{}
	};

	/* Defines a color component mapping. */
	struct ComponentMapping
	{
	public:
		/* Specifies the component value placed in the R component. */
		ComponentSwizzle R;
		/* Specifies the component value placed in the G component. */
		ComponentSwizzle G;
		/* Specifies the component value placed in the B component. */
		ComponentSwizzle B;
		/* Specifies the component value placed in the A component. */
		ComponentSwizzle A;

		/* Initializes a default instance of a component mapping. */
		ComponentMapping(void)
			: ComponentMapping(ComponentSwizzle::Identity, ComponentSwizzle::Identity, ComponentSwizzle::Identity, ComponentSwizzle::Identity)
		{}

		/* Initializes a new instance of a component mapping with all components specified. */
		ComponentMapping(_In_ ComponentSwizzle r, _In_ ComponentSwizzle g, _In_ ComponentSwizzle b, _In_ ComponentSwizzle a)
			: R(r), G(g), B(b), A(a)
		{}
	};

	/* Defines the information required to create a image view. */
	struct ImageViewCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the image on which the view will be created. */
		ImageHndl Image;
		/*Specifies the type of image view.  */
		ImageViewType ViewType;
		/* Specifies the format and type used to interpret texel blocks. */
		Format Format;
		/* Specifies a remapping of color components. */
		ComponentMapping Components;
		/* Specifies the set of mipmap levels and array layers to be accessible to the view. */
		ImageSubresourceRange SubresourceRange;

		/* Initializes an empty instance of the image view create info object. */
		ImageViewCreateInfo(void)
			: ImageViewCreateInfo(nullptr, ImageViewType::Image2D, Format::Undefined, ImageAspectFlag::Color)
		{}

		/* Initializes a new instance of an image view create info object. */
		ImageViewCreateInfo(_In_ ImageHndl image, _In_ ImageViewType type, _In_ Pu::Format format, _In_ ImageAspectFlag aspect)
			: Type(StructureType::ImageViewCreateInfo), Next(nullptr), Flags(0),
			Image(image), ViewType(type), Format(format), Components(), SubresourceRange(aspect)
		{}
	};

	/* Defines the information required to create a framebuffer. */
	struct FramebufferCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies what renderpass the framebuffer will be compatible with. */
		RenderPassHndl RenderPass;
		/* Specifies the amount of image view attachments. */
		uint32 AttachmentCount;
		/* Specifies the image views corresponding to the render pass attachments. */
		const ImageViewHndl *Attachments;
		/* Specifies the width of the framebuffer. */
		uint32 Width;
		/* Specifies the height of the framebuffer. */
		uint32 Height;
		/* Specifies the layers of the framebuffer. */
		uint32 Layers;

		/* Initializes an empty instance of the framebuffer create info object. */
		FramebufferCreateInfo(void)
			: FramebufferCreateInfo(nullptr, 0, 0)
		{}

		/* Initializes a new instance of the framebuffer create info object. */
		FramebufferCreateInfo(_In_ RenderPassHndl renderpass, _In_ uint32 width, _In_ uint32 height)
			: Type(StructureType::FramebufferCreateInfo), Next(nullptr), Flags(0),
			RenderPass(renderpass), AttachmentCount(0), Attachments(nullptr),
			Width(width), Height(height), Layers(1)
		{}
	};

	/* Defines a specialization map entry. */
	struct SpecializationMapEntry
	{
	public:
		/* Specifies the ID of the specialization constant in SPIR-V. */
		uint32 ConstantID;
		/* Specifies the offset (in bytes) of the constant value within the supplied data buffer. */
		uint32 Offset;
		/* Specifies the size (in bytes) of the constant value within the supplied data buffer. */
		size_t Size;

		/* Initializes an empty instance of a specialization map entry object. */
		SpecializationMapEntry(void)
			: SpecializationMapEntry(0, 0, 0)
		{}

		/* Initializes a new instance of a specialization map entry object. */
		SpecializationMapEntry(_In_ uint32 id, _In_ uint32 offset, _In_ size_t size)
			: ConstantID(id), Offset(offset), Size(size)
		{}
	};

	/* Defines information about specialization. */
	struct SpecializationInfo
	{
	public:
		/* Specifies the number of map entries. */
		uint32 MapEntryCount;
		/* Specifies all entries in the supplied data buffer. */
		const SpecializationMapEntry *MapEntries;
		/* Specifies the size (in bytes) of the supplied data buffer. */
		size_t DataSize;
		/* Specifies the actual constant value to specialize with. */
		const void *Data;

		/* Initializes an ampty instance of the specialization info object. */
		SpecializationInfo(void)
			: SpecializationInfo(0, nullptr)
		{}

		/* Initializes a new instance of the specialization info object. */
		SpecializationInfo(_In_ size_t size, _In_ const void *data)
			: MapEntryCount(0), MapEntries(nullptr), DataSize(size), Data(data)
		{}
	};

	/* Defines the information of a pipeline shader stage. */
	struct PipelineShaderStageCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the single pipeline stage. */
		ShaderStageFlag Stage;
		/* Specifies the object that contains the shader for this stage. */
		ShaderModuleHndl Module;
		/* Specifies a null-terminated UTF-8 string with the entry point name of the shader. */
		const char *Name;
		/* Specifies optional specialization information. */
		const SpecializationInfo *SpecializationInfo;

		/* Initializes a empty instance of the pipeline shader stage create info object. */
		PipelineShaderStageCreateInfo(void)
			: PipelineShaderStageCreateInfo(ShaderStageFlag::Unknown, nullptr)
		{}

		/* Initializes a new instance of the pipeline shader stage create info object. */
		PipelineShaderStageCreateInfo(_In_ ShaderStageFlag stage, _In_ ShaderModuleHndl moduleHndl)
			: Type(StructureType::PipelineShaderStageCreateInfo), Next(nullptr), Flags(0),
			Stage(stage), Module(moduleHndl), Name("main"), SpecializationInfo(nullptr)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineShaderStageCreateInfo& operator =(_In_ const PipelineShaderStageCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Stage = other.Stage;
				Module = other.Module;
				Name = other.Name;
				SpecializationInfo = other.SpecializationInfo;
			}

			return *this;
		}
	};

	/* Defines how a vertex input should be bound. */
	struct VertexInputBindingDescription
	{
	public:
		/* Specifies the binding number of the vertex input. */
		uint32 Binding;
		/* Specifies the distance (in bytes) between two consecutive elements. */
		uint32 Stride;
		/* Specifies whether the vertex attribute addressing is bound to the vertex index or the instance index. */
		VertexInputRate InputRate;

		/* Initializes an empty instance of a vertex input binding description. */
		VertexInputBindingDescription(void)
			: VertexInputBindingDescription(0, 0, VertexInputRate::Vertex)
		{}

		/* Initializes a new instance of a vertex input binding description. */
		VertexInputBindingDescription(_In_ uint32 binding, _In_ uint32 stride, _In_ VertexInputRate inputRate)
			: Binding(binding), Stride(stride), InputRate(inputRate)
		{}
	};

	/* Defines the attributes of a vertex input. */
	struct VertexInputAttributeDescription
	{
	public:
		/* Specifies the location number of this attribute. */
		uint32 Location;
		/* Specifies the binding number of the attribute. */
		uint32 Binding;
		/* Specifies the size and type of the vertex attribute. */
		Format Format;
		/* Specifies the offset (in bytes) of the relative start of an element. */
		uint32 Offset;

		/* Initializes an empty instance of the vertex input attribute description object. */
		VertexInputAttributeDescription(void)
			: VertexInputAttributeDescription(0, 0, Format::Undefined, 0)
		{}

		/* Initializes a new instance of the vertex input attribute description object. */
		VertexInputAttributeDescription(_In_ uint32 location, _In_ uint32 binding, _In_ Pu::Format format, _In_ uint32 offset)
			: Location(location), Binding(binding), Format(format), Offset(offset)
		{}
	};

	/* Defines the information of a pipeline vertex input. */
	struct PipelineVertexInputStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the amount of vertex binding descriptions provided. */
		uint32 VertexBindingDescriptionCount;
		/* Specifies the vertex bindings. */
		const VertexInputBindingDescription *VertexBindingDescriptions;
		/* Specifies the amount of vertex attribute descriptions provided. */
		uint32 VertexAttributeDescriptionCount;
		/* Specifies the vertex attributes. */
		const VertexInputAttributeDescription *VertexAttributeDescriptions;

		/* Initializes an empty instance of a pipeline vertex input state create info object. */
		PipelineVertexInputStateCreateInfo(void)
			: Type(StructureType::PipelineVertexInputStateCreateInfo), Next(nullptr), Flags(0),
			VertexBindingDescriptionCount(0), VertexBindingDescriptions(nullptr),
			VertexAttributeDescriptionCount(0), VertexAttributeDescriptions(nullptr)
		{}

		/* Copy contructor. */
		PipelineVertexInputStateCreateInfo(_In_ const PipelineVertexInputStateCreateInfo &value)
			: Type(StructureType::PipelineVertexInputStateCreateInfo), Next(value.Next), Flags(value.Flags),
			VertexBindingDescriptionCount(value.VertexBindingDescriptionCount), 
			VertexBindingDescriptions(value.VertexBindingDescriptions),
			VertexAttributeDescriptionCount(value.VertexAttributeDescriptionCount), 
			VertexAttributeDescriptions(value.VertexAttributeDescriptions)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineVertexInputStateCreateInfo& operator =(_In_ const PipelineVertexInputStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				VertexAttributeDescriptionCount = other.VertexBindingDescriptionCount;
				VertexBindingDescriptions = other.VertexBindingDescriptions;
				VertexAttributeDescriptionCount = other.VertexAttributeDescriptionCount;
				VertexAttributeDescriptions = other.VertexAttributeDescriptions;
			}

			return *this;
		}
	};

	/* Defines the information of a pipeline assembly state. */
	struct PipelineInputAssemblyStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies which primitve topology to use. */
		PrimitiveTopology Topology;
		/* Specifies whether a special index value is treated as restarting the assembly of primitives. */
		Bool32 PrimitiveRestartEnable;

		/* Initializes an empty instance of the pipeline input assembly state create info object.  */
		PipelineInputAssemblyStateCreateInfo(void)
			: PipelineInputAssemblyStateCreateInfo(PrimitiveTopology::PointList)
		{}

		/* Initializes a new instance of the pipeline input assembly state create info object. */
		PipelineInputAssemblyStateCreateInfo(_In_ PrimitiveTopology topology)
			: Type(StructureType::PipelineInputAssemblyStateCreateInfo), Next(nullptr), Flags(0),
			Topology(topology), PrimitiveRestartEnable(false)
		{}

		/* Copy constructor. */
		PipelineInputAssemblyStateCreateInfo(_In_ const PipelineInputAssemblyStateCreateInfo &value)
			: Type(StructureType::PipelineInputAssemblyStateCreateInfo), Next(value.Next),
			Flags(value.Flags), Topology(value.Topology), PrimitiveRestartEnable(value.PrimitiveRestartEnable)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineInputAssemblyStateCreateInfo& operator =(_In_ const PipelineInputAssemblyStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				Topology = other.Topology;
				PrimitiveRestartEnable = other.PrimitiveRestartEnable;
			}

			return *this;
		}
	};

	/* Defines the information for a pipeline tessellation state. */
	struct PipelineTessellationStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* The number of control points per patch. */
		uint32 PathControlPoints;

		/* Initializes an empty instance of the pipeline tessellation state create info object. */
		PipelineTessellationStateCreateInfo(void)
			: PipelineTessellationStateCreateInfo(0)
		{}

		/* Initializes a new instance of a pipline tessellation state create info object. */
		PipelineTessellationStateCreateInfo(_In_ uint32 pathControlPoints)
			: Type(StructureType::PipelineTessellationStateCreateInfo), Next(nullptr),
			Flags(0), PathControlPoints(pathControlPoints)
		{}

		/* Copy constructor. */
		PipelineTessellationStateCreateInfo(_In_ const PipelineTessellationStateCreateInfo &value)
			: Type(StructureType::PipelineTessellationStateCreateInfo), Next(value.Next),
			Flags(value.Flags), PathControlPoints(value.PathControlPoints)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineTessellationStateCreateInfo& operator =(_In_ const PipelineTessellationStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				PathControlPoints = other.PathControlPoints;
			}

			return *this;
		}
	};

	/* Defines a two-dimensional offset. */
	struct Offset2D
	{
	public:
		/* The horizontal component. */
		int32 X;
		/* The vertical component. */
		int32 Y;

		/* Initializes an empty instance of an offset. */
		Offset2D(void)
			: Offset2D(0, 0)
		{}

		/* Initializes a new instance of an offset. */
		Offset2D(_In_ int32 x, _In_ int32 y)
			: X(x), Y(y)
		{}
	};

	/* Defines a three-dimensional offset. */
	struct Offset3D
	{
	public:
		/* The horizontal component. */
		int32 X;
		/* The vertical component. */
		int32 Y;
		/* The depth component. */
		int32 Z;

		/* Initializes an empty instance of an offset. */
		Offset3D(void)
			: Offset3D(0, 0, 0)
		{}

		/* Initializes a new instance of an offset. */
		Offset3D(_In_ int32 x, _In_ int32 y, _In_ int32 z)
			: X(x), Y(y), Z(z)
		{}

		/* Initializes a new instance of an offset from an extent. */
		Offset3D(_In_ Extent3D extent)
			: X(static_cast<int32>(extent.Width)), Y(static_cast<int32>(extent.Height)), Z(static_cast<int32>(extent.Depth))
		{}

		/* Divides the offset by a specific amount. */
		_Check_return_ inline Offset3D operator /=(_In_ int scalar)
		{
			X /= scalar;
			Y /= scalar;
			Z /= scalar;
			return *this;
		}
	};

	/* Defines a two-dimensional subregion. */
	struct Rect2D
	{
	public:
		/* Specifies the rectangles offset. */
		Offset2D Offset;
		/* Specifies the rectnagles extent. */
		Extent2D Extent;

		/* Initializes an empty instance of a rectangle. */
		Rect2D(void)
			: Rect2D(0, 0, 0, 0)
		{}

		/* Initializes a new instance of a rectangle. */
		Rect2D(_In_ Extent2D extent)
			: Rect2D(Offset2D(), extent)
		{}

		/* Initializes a new instance of a rectangle. */
		Rect2D(_In_ uint32 w, _In_ uint32 h)
			: Rect2D(0, 0, w, h)
		{}

		/* Initializes a new instance of a rectangle. */
		Rect2D(_In_ Offset2D offset, _In_ Extent2D extent)
			: Offset(offset), Extent(extent)
		{}

		/* Initializes a new instance of a rectangle. */
		Rect2D(_In_ int32 x, _In_ int32 y, _In_ uint32 w, _In_ uint32 h)
			: Offset(x, y), Extent(w, h)
		{}

		/* Checks whether the specified point is inside the rectangle. */
		_Check_return_ inline bool Contains(_In_ Offset2D point) const
		{
			const int32 left = Offset.X;
			const int32 right = left + Extent.Width;
			const int32 top = Offset.Y;
			const int32 bottom = top + Extent.Height;

			return left <= point.X && right >= point.X && top <= point.Y && bottom >= point.Y;
		}
	};

	/* Defines a Vulkan compatible viewport. */
	struct Viewport
	{
	public:
		/* Specifies left left-most coordinate of the viewport. */
		float X;
		/* Specifies the top-most coordinate of the viewport. */
		float Y;
		/* Specifies the width of the viewport. */
		float Width;
		/* Specifies the height of the viewport. */
		float Height;
		/* Specifies the minimum depth range of the viewport. */
		float MinDepth;
		/* Specifies the maximum depth range of the viewport. */
		float MaxDepth;

		/* Initializes an empty instance of a viewport. */
		Viewport(void)
			: Viewport(0.0f, 0.0f)
		{}

		/* Initializes a new instance of a viewport object. */
		Viewport(_In_ float w, _In_ float h)
			: X(0.0f), Y(0.0f), Width(w), Height(h), MinDepth(0.0f), MaxDepth(1.0f)
		{}

		/* Initializes a new instance of a viewport object. */
		Viewport(_In_ Rect2D size)
			: X(static_cast<float>(size.Offset.X)), Y(static_cast<float>(size.Offset.Y)), 
			Width(static_cast<float>(size.Extent.Width)), Height(static_cast<float>(size.Extent.Height)),
			MinDepth(0.0f), MaxDepth(1.0f)
		{}

		/* Gets the position of the viewport. */
		_Check_return_ inline Offset2D GetPosition(void) const
		{
			return Offset2D(static_cast<int32>(X), static_cast<int32>(Y));
		}

		/* Gets the size of the viewport. */
		_Check_return_ inline Extent2D GetSize(void) const
		{
			return Extent2D(static_cast<uint32>(Width), static_cast<uint32>(Height));
		}

		/* Gets the default scissor area for this viewport. */
		_Check_return_ inline Rect2D GetScissor(void) const
		{
			return Rect2D(static_cast<int32>(X), static_cast<int32>(Y), static_cast<uint32>(Width), static_cast<uint32>(Height));
		}
	};

	/* Defines the information for a pipeline viewport. */
	struct PipelineViewportStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the number of viewports used by the pipeline. */
		uint32 ViewportCount;
		/* Specifies the viewports used by the pipeline. */
		const Viewport *Viewports;
		/* Specifies the number of scissors used by the pipeline. */
		uint32 ScissorCount;
		/* Specifies the scissors used by the pipeline. */
		const Rect2D *Scissors;

		/* Initializes an empty instance of a pipeline viewport state create info object. */
		PipelineViewportStateCreateInfo(void)
			: Type(StructureType::PipelineViewportStateCreateInfo), Next(nullptr), Flags(0),
			ViewportCount(0), Viewports(nullptr), ScissorCount(0), Scissors(nullptr)
		{}

		/* Initializes a new instance of a pipeline viewport state create info object. */
		PipelineViewportStateCreateInfo(_In_ const Viewport &viewport, _In_ const Rect2D &scissor)
			: Type(StructureType::PipelineViewportStateCreateInfo), Next(nullptr), Flags(0),
			ViewportCount(1), Viewports(&viewport), ScissorCount(1), Scissors(&scissor)
		{}

		/* Copy constructor. */
		PipelineViewportStateCreateInfo(_In_ const PipelineViewportStateCreateInfo &value)
			: Type(StructureType::PipelineViewportStateCreateInfo), Next(value.Next), 
			Flags(value.Flags), ViewportCount(value.ViewportCount), 
			Viewports(value.Viewports), ScissorCount(value.ScissorCount), Scissors(value.Scissors)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineViewportStateCreateInfo& operator =(_In_ const PipelineViewportStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				ViewportCount = other.ViewportCount;
				Viewports = other.Viewports;
				ScissorCount = other.ScissorCount;
				Scissors = other.Scissors;
			}

			return *this;
		}
	};

	/* Defines the information for a pipeline rasterizer. */
	struct PipelineRasterizationStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies whether depth values outside the depth range should be rasterized. */
		Bool32 DepthClampEnable;
		/* Specifies whether to diactivate fragment generation. */
		Bool32 RasterizerDiscardEnable;
		/* Specifies how a polygon should be rasterized. */
		PolygonMode PolygonMode;
		/* Specifies which polygons to cull. */
		CullModeFlag CullMode;
		/* Specifies what to consider the front face of a polygon. */
		FrontFace FrontFace;
		/* Specifies whether depth biasing is enabled. */
		Bool32 DepthBiasEnable;
		/* Specifies the constant factor added to each depth value when biasing is enabled. */
		float DepthBiasConstantFactor;
		/* Specifies the maximum value of bias that can be applied to a depth value. */
		float DepthBiasClamp;
		/* Specifies the slope factor applied to depth when biasing is enabled. */
		float DepthBiasSlopeFactor;
		/* Specifies the width of rasterized lines. */
		float LineWidth;

		/* Initializes an empty instance of the pipeline rasterization state create info object. */
		PipelineRasterizationStateCreateInfo(void)
			: PipelineRasterizationStateCreateInfo(CullModeFlag::None)
		{}

		/* Initializes a new instance of the pipeline rasterization state create info object. */
		PipelineRasterizationStateCreateInfo(_In_ CullModeFlag cullMode)
			: Type(StructureType::PipelineRasterizationStateCreateInfo), Next(nullptr), Flags(0),
			DepthClampEnable(false), RasterizerDiscardEnable(false), PolygonMode(PolygonMode::Fill),
			CullMode(cullMode), FrontFace(FrontFace::CounterClockwise), DepthBiasEnable(false),
			DepthBiasConstantFactor(0.0f), DepthBiasClamp(0.0f), DepthBiasSlopeFactor(0.0f), LineWidth(1.0f)
		{}

		/* Copy constructor. */
		PipelineRasterizationStateCreateInfo(_In_ const PipelineRasterizationStateCreateInfo &value)
			: Type(StructureType::PipelineRasterizationStateCreateInfo), Next(value.Next), Flags(value.Flags),
			DepthClampEnable(value.DepthClampEnable), RasterizerDiscardEnable(value.RasterizerDiscardEnable), 
			PolygonMode(value.PolygonMode), CullMode(value.CullMode), FrontFace(value.FrontFace), 
			DepthBiasEnable(value.DepthBiasEnable), DepthBiasConstantFactor(value.DepthBiasConstantFactor), 
			DepthBiasClamp(value.DepthBiasClamp), DepthBiasSlopeFactor(value.DepthBiasSlopeFactor), LineWidth(value.LineWidth)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineRasterizationStateCreateInfo& operator =(_In_ const PipelineRasterizationStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				DepthClampEnable = other.DepthClampEnable;
				RasterizerDiscardEnable = other.RasterizerDiscardEnable;
				PolygonMode = other.PolygonMode;
				CullMode = other.CullMode;
				FrontFace = other.FrontFace;
				DepthBiasEnable = other.DepthBiasEnable;
				DepthBiasConstantFactor = other.DepthBiasConstantFactor;
				DepthBiasClamp = other.DepthBiasClamp;
				DepthBiasSlopeFactor = other.DepthBiasSlopeFactor;
				LineWidth = other.LineWidth;
			}

			return *this;
		}
	};

	/* Defines the information for a pipeline multisample state. */
	struct PipelineMultisampleStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the number of samples to use per pixel. */
		SampleCountFlag RasterizationSamples;
		/* Specifies whether shading should occur per sample rather than per fragment. */
		Bool32 SampleShading;
		/* Specifies the minimum number of unique sample locations to use during fragment shading. */
		float MinSampleShading;
		/* Specifies static coverage sample masks. */
		const SampleMask *SampleMask;
		/* Specifies whether fragment's alpha value should be used for coverage. */
		Bool32 AlphaToCoverageEnable;
		/* Specifies whether fragment's alpha value should be replaced with one. */
		Bool32 AlphaToOneEnable;

		/* Initializes an empty instance of a pipeline multisample state create info object. */
		PipelineMultisampleStateCreateInfo(void)
			: PipelineMultisampleStateCreateInfo(SampleCountFlag::Pixel1Bit)
		{}

		/* Initializes a new instance of a pipeline multisample create info object. */
		PipelineMultisampleStateCreateInfo(_In_ SampleCountFlag samples)
			: Type(StructureType::PipelineMultiSampleStateCreateInfo), Next(nullptr), Flags(0),
			RasterizationSamples(samples), SampleShading(false), MinSampleShading(0.0f),
			SampleMask(nullptr), AlphaToCoverageEnable(false), AlphaToOneEnable(false)
		{}

		/* Copy constructor. */
		PipelineMultisampleStateCreateInfo(_In_ const PipelineMultisampleStateCreateInfo &value)
			: Type(StructureType::PipelineMultiSampleStateCreateInfo), Next(value.Next), 
			Flags(value.Flags), RasterizationSamples(value.RasterizationSamples), 
			SampleShading(value.SampleShading), MinSampleShading(value.MinSampleShading),
			SampleMask(value.SampleMask), AlphaToCoverageEnable(value.AlphaToCoverageEnable),
			AlphaToOneEnable(value.AlphaToOneEnable)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineMultisampleStateCreateInfo& operator =(_In_ const PipelineMultisampleStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				RasterizationSamples = other.RasterizationSamples;
				SampleShading = other.SampleShading;
				MinSampleShading = other.MinSampleShading;
				SampleMask = other.SampleMask;
				AlphaToCoverageEnable = other.AlphaToCoverageEnable;
				AlphaToOneEnable = other.AlphaToOneEnable;
			}

			return *this;
		}
	};

	/* Defines how stencil testing should be performed. */
	struct StencilOpState
	{
	public:
		/* Specifies what action to perform on samples that fail the stencil test. */
		StencilOp FailOp;
		/* Specifies what action to perform on samples that pass both the depth and stencil test. */
		StencilOp PassOp;
		/* Specifies what action to perform on samples that fail the depth test. */
		StencilOp DepthFailOp;
		/* Specifies the comparison operator used in the stencil test. */
		CompareOp CompareOp;
		/* Specifies which bits of the stencil value participate in the stencil test. */
		uint32 CompareMask;
		/* Specifies which bits of the stencil value are updated by the stencil test. */
		uint32 WriteMask;
		/* Specifies an unsigned integer used in stencil comparison. */
		uint32 Reference;

		/* Initializes an empty instance of the stencil operation state object. */
		StencilOpState(void)
			: FailOp(StencilOp::Keep), PassOp(StencilOp::Keep), DepthFailOp(StencilOp::Keep),
			CompareOp(CompareOp::Never), CompareMask(0), WriteMask(0), Reference(0)
		{}
	};

	/* Defines the information for a pipeline depth/stencil state. */
	struct PipelineDepthStencilStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies whether depth testing is enabled. */
		Bool32 DepthTestEnable;
		/* Specifies whether writing depth values is enabled (this is always disabled is DepthTestEnable is false). */
		Bool32 DepthWriteEnable;
		/* Specifies the comparison operator used in depth testing. */
		CompareOp DepthCompareOp;
		/* Specifies whether depth bounds testing is enabled. */
		Bool32 DepthBoundsTestEnable;
		/* Specifies whether stencil testing is enabled. */
		Bool32 StencilTestEnable;
		/* Specifies how the stencil test should be performed for front facing primitives. */
		StencilOpState Front;
		/* Specifies how the stencil test should be performed for back facing primitives. */
		StencilOpState Back;
		/* Specifies the minimum value for depth bounds testing. */
		float MinDepthBounds;
		/* Specifies the maximum value for depth bounds testing. */
		float MaxDepthBounds;

		/* Initializes an empty instance of the pipeline depth/stencil state create info object. */
		PipelineDepthStencilStateCreateInfo(void)
			: Type(StructureType::PipelineDepthStencilStateCreateInfo), Next(nullptr), Flags(0),
			DepthTestEnable(true), DepthWriteEnable(true), DepthCompareOp(CompareOp::LessOrEqual),
			DepthBoundsTestEnable(false), StencilTestEnable(false), MinDepthBounds(0.0f), MaxDepthBounds(0.0f)
		{}

		/* Copy constructor. */
		PipelineDepthStencilStateCreateInfo(_In_ const PipelineDepthStencilStateCreateInfo &value)
			: Type(StructureType::PipelineDepthStencilStateCreateInfo), Next(value.Next), Flags(value.Flags),
			DepthTestEnable(value.DepthTestEnable), DepthWriteEnable(value.DepthWriteEnable), 
			DepthCompareOp(value.DepthCompareOp), DepthBoundsTestEnable(value.DepthBoundsTestEnable),
			StencilTestEnable(value.StencilTestEnable), MinDepthBounds(value.MinDepthBounds), MaxDepthBounds(value.MaxDepthBounds)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineDepthStencilStateCreateInfo& operator =(_In_ const PipelineDepthStencilStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				DepthTestEnable = other.DepthTestEnable;
				DepthWriteEnable = other.DepthWriteEnable;
				DepthCompareOp = other.DepthCompareOp;
				DepthBoundsTestEnable = other.DepthBoundsTestEnable;
				StencilTestEnable = other.StencilTestEnable;
				MinDepthBounds = other.MinDepthBounds;
				MaxDepthBounds = other.MaxDepthBounds;
			}

			return *this;
		}
	};

	/* Defines the information for a pipeline color blend attachment state. */
	struct PipelineColorBlendAttachmentState
	{
	public:
		/* Specifies if blending is enabled. */
		Bool32 BlendEnable;
		/* Specifies the blending factor for the incoming color. */
		BlendFactor SrcColorBlendFactor;
		/* Specifies the blending for the stored color. */
		BlendFactor DstColorBlendFactor;
		/* Specifies the operation to perform on the color. */
		BlendOp ColorBlendOp;
		/* Specifies the blending factor for the incoming alpha. */
		BlendFactor SrcAlphaBlendFactor;
		/* Specifies the blending factor for the stored alpha. */
		BlendFactor DstAlphaBlendFactor;
		/* Specifies the operation to perform on the alpha. */
		BlendOp AlphaBlendOp;
		/* Specifies which components are enabled for writing. */
		ColorComponentFlag ColorWriteMask;

		/* Initializes an empty instance of a pipeline color blend attachment state. */
		PipelineColorBlendAttachmentState(void)
			: BlendEnable(false), ColorWriteMask(ColorComponentFlag::RGBA),
			SrcColorBlendFactor(BlendFactor::One), DstColorBlendFactor(BlendFactor::Zero), ColorBlendOp(BlendOp::Add),
			SrcAlphaBlendFactor(BlendFactor::One), DstAlphaBlendFactor(BlendFactor::Zero), AlphaBlendOp(BlendOp::Add)
		{}
	};

	/* Defines the information for a pipeline color blend state. */
	struct PipelineColorBlendStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies whether logical operations are enabled. */
		Bool32 LogicOpEnable;
		/* Specifies the logical operation to perform. */
		LogicOp LogicOp;
		/* Specifies the amount of attachments specified. */
		uint32 AttachmentCount;
		/* Specifies the parameters for each color blending attachment. */
		const PipelineColorBlendAttachmentState *Attachments;
		/* Specifies the constant color used for some blend factors. */
		float BlendConstants[4];

		/* Initializes an empty instance of the pipeline color blend state create info object. */
		PipelineColorBlendStateCreateInfo(void)
			: Type(StructureType::PipelineColorBlendStateCreateInfo), Next(nullptr), Flags(0),
			LogicOpEnable(false), LogicOp(LogicOp::Copy), AttachmentCount(0), Attachments(nullptr),
			BlendConstants{ 0.0f, 0.0f, 0.0f, 0.0f }
		{}

		/* Initializes a new instance of the pipeline color blend state create info object. */
		PipelineColorBlendStateCreateInfo(_In_ const vector<PipelineColorBlendAttachmentState> &states)
			: Type(StructureType::PipelineColorBlendStateCreateInfo), Next(nullptr), Flags(0),
			LogicOpEnable(false), LogicOp(LogicOp::Copy), AttachmentCount(static_cast<uint32>(states.size())), 
			Attachments(states.data()), BlendConstants{0.0f, 0.0f, 0.0f, 0.0f}
		{}

		/* Copy constructor. */
		PipelineColorBlendStateCreateInfo(_In_ const PipelineColorBlendStateCreateInfo &value)
			: Type(StructureType::PipelineColorBlendStateCreateInfo), Next(value.Next), 
			Flags(value.Flags), LogicOpEnable(value.LogicOpEnable), LogicOp(value.LogicOp), 
			AttachmentCount(value.AttachmentCount), Attachments(value.Attachments)
		{
			memcpy(BlendConstants, value.BlendConstants, sizeof(BlendConstants));
		}

		/* Copy assignment. */
		_Check_return_ inline PipelineColorBlendStateCreateInfo& operator =(_In_ const PipelineColorBlendStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				LogicOpEnable = other.LogicOpEnable;
				LogicOp = other.LogicOp;
				AttachmentCount = other.AttachmentCount;
				Attachments = other.Attachments;
			}

			return *this;
		}
	};

	/* Defines the information for a pipeline dynamic state. */
	struct PipelineDynamicStateCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the amount of elements in the DynamicStates field. */
		uint32 DynamicStateCount;
		/* Specifies which pieces of pipeline state will use the values from dynamic state commands. */
		const DynamicState *DynamicStates;

		/* Initializes an empty instance of the pipeline dynamic state create info object. */
		PipelineDynamicStateCreateInfo(void)
			: Type(StructureType::PipelineDynamicStateCreateInfo), Next(nullptr), Flags(0),
			DynamicStateCount(0), DynamicStates(nullptr)
		{}

		/* Initializes a new instance of a pipeline dynamic state create info object. */
		PipelineDynamicStateCreateInfo(_In_ const vector<DynamicState> &dynamicStates)
			: Type(StructureType::PipelineDynamicStateCreateInfo), Next(nullptr), Flags(0),
			DynamicStateCount(static_cast<uint32>(dynamicStates.size())), DynamicStates(dynamicStates.data())
		{}

		/* Copy constructor. */
		PipelineDynamicStateCreateInfo(_In_ const PipelineDynamicStateCreateInfo &value)
			: Type(StructureType::PipelineDynamicStateCreateInfo), Next(value.Next), Flags(value.Flags), 
			DynamicStateCount(value.DynamicStateCount), DynamicStates(value.DynamicStates)
		{}

		/* Copy assignment. */
		_Check_return_ inline PipelineDynamicStateCreateInfo& operator =(_In_ const PipelineDynamicStateCreateInfo &other)
		{
			if (this != &other)
			{
				Next = other.Next;
				Flags = other.Flags;
				DynamicStateCount = other.DynamicStateCount;
				DynamicStates = other.DynamicStates;
			}

			return *this;
		}
	};

	/* Defines a push constant range. */
	struct PushConstantRange
	{
	public:
		/* Specifies the shader stages that will acces a range of push constants. */
		ShaderStageFlag StageFlags;
		/* Specifies the offset (in bytes) of the range (must be a multiple of 4!). */
		uint32 Offset;
		/* Specifies the size (in bytes) of the range (must be a multiple of 4!). */
		uint32 Size;

		/* Initializes an empty instance of a push constant range object. */
		PushConstantRange(void)
			: PushConstantRange(ShaderStageFlag::Unknown, 0, 0)
		{}

		/* Initializes a new instance of a push constant range object. */
		PushConstantRange(_In_ ShaderStageFlag stages, _In_ uint32 offset, _In_ uint32 size)
			: StageFlags(stages), Offset(offset), Size(size)
		{}
	};

	/* Defines the information for a pipeline layout. */
	struct PipelineLayoutCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the amount of set layouts provided. */
		uint32 SetLayoutCount;
		/* Specifies the desciptor set layouts. */
		const DescriptorSetLayoutHndl *SetLayouts;
		/* Specifies the amount of push constants provided. */
		uint32 PushConstantRangeCount;
		/* Specifies the push constants. */
		const PushConstantRange *PushConstantRanges;

		/* Initializes an empty instance of the pipeline layout create info object. */
		PipelineLayoutCreateInfo(void)
			: Type(StructureType::PipelineLayourCreateInfo), Next(nullptr), Flags(0),
			SetLayoutCount(0), SetLayouts(nullptr), PushConstantRangeCount(0), PushConstantRanges(nullptr)
		{}

		/* Initializes a new instance of the pipeline layout create info object. */
		PipelineLayoutCreateInfo(_In_ const vector<DescriptorSetLayoutHndl>& descriptorSets, _In_ const vector<PushConstantRange> &pushConstants)
			: Type(StructureType::PipelineLayourCreateInfo), Next(nullptr), Flags(0),
			SetLayoutCount(static_cast<uint32>(descriptorSets.size())), SetLayouts(descriptorSets.data()), 
			PushConstantRangeCount(static_cast<uint32>(pushConstants.size())), PushConstantRanges(pushConstants.data())
		{}
	};

	/* Defines the information required to create a graphics pipeline. */
	struct GraphicsPipelineCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies how the pipeline should be generated. */
		PipelineCreateFlag Flags;
		/* Specifies the amount of entries in the Stages field. */
		uint32 StageCount;
		/* Specifies the shader stages included in the pipeline. */
		const PipelineShaderStageCreateInfo *Stages;
		/* Specifies the vertex input for the pipeline (ignored if a mesh shader stage is present). */
		const PipelineVertexInputStateCreateInfo *VertexInputState;
		/* Specifies the input assembly behaviour. */
		const PipelineInputAssemblyStateCreateInfo *InputAssemblyState;
		/* Specifies aditional tessellation information (ignored if no tessellation control and evaluation stage are present). */
		const PipelineTessellationStateCreateInfo *TessellationState;
		/* Specifies the pipelines viewport (ignored if rasterization is disabled). */
		const PipelineViewportStateCreateInfo *ViewportState;
		/* Specifies how the pipeline should rasterize. */
		const PipelineRasterizationStateCreateInfo *RasterizationState;
		/* Specifies how the pipeline should multisample (ignored if rasterization is disabled). */
		const PipelineMultisampleStateCreateInfo *MultisampleState;
		/* Specifies how depth and stencil attachments should be used (ignored if rasterization is disabled or if no depth/stencil attachment is used). */
		const PipelineDepthStencilStateCreateInfo *DepthStencilState;
		/* Specifies how color blending should be handled (ignored if rasterization is diabled or if no color attachment is used). */
		const PipelineColorBlendStateCreateInfo *ColorBlendState;
		/* Specifies the point at which the pipeline can be dynamically changed. */
		const PipelineDynamicStateCreateInfo *DynamicState;
		/* Specifies the binding locations used by both the pipeline and descriptor sets. */
		PipelineLayoutHndl Layout;
		/* Specifies the renderpass enviroment for the pipeline. */
		RenderPassHndl Renderpass;
		/* Specifies the index of the subpass in the render pass where this pipeline will be used. */
		uint32 Subpass;
		/* Specifies the parent pipeline. */
		PipelineHndl BasePipelineHandle;
		/* Specifies which create info to derive from. */
		int32 BasePipelineIndex;

		/* Initializes an empty instance of a graphics pipeline create info object. */
		GraphicsPipelineCreateInfo(void)
			: Type(StructureType::GraphicsPipelineCreateInfo), Next(nullptr), Flags(PipelineCreateFlag::None),
			StageCount(0), Stages(nullptr), VertexInputState(nullptr), InputAssemblyState(nullptr), TessellationState(nullptr), ViewportState(nullptr),
			RasterizationState(nullptr), MultisampleState(nullptr), DepthStencilState(nullptr), ColorBlendState(nullptr),
			DynamicState(nullptr), Layout(nullptr), Renderpass(nullptr), Subpass(0), BasePipelineHandle(nullptr), BasePipelineIndex(-1)
		{}

		/* Initializes a new instance of a graphics pipeline create info object. */
		GraphicsPipelineCreateInfo(_In_ const vector<PipelineShaderStageCreateInfo> &stages, _In_ PipelineLayoutHndl layout, _In_ RenderPassHndl renderpass, _In_ uint32 subpass)
			: Type(StructureType::GraphicsPipelineCreateInfo), Next(nullptr), Flags(PipelineCreateFlag::None),
			StageCount(static_cast<uint32>(stages.size())), Stages(stages.data()), VertexInputState(nullptr),
			InputAssemblyState(nullptr), ViewportState(nullptr), TessellationState(nullptr), RasterizationState(nullptr), 
			MultisampleState(nullptr), DepthStencilState(nullptr), ColorBlendState(nullptr), DynamicState(nullptr),
			Layout(layout), Renderpass(renderpass), Subpass(subpass), BasePipelineHandle(nullptr), BasePipelineIndex(-1)
		{}
	};

	/* Defines the value used to clear a depth/stencil buffer. */
	struct ClearDepthStencilValue
	{
	public:
		/* Specifies the clear value for the depth aspect. */
		float Depth;
		/* Specifies the clear value for the stencil aspect. */
		uint32 Stencil;

		/* Initializes an empty instance of the clear depth/stencil object. */
		ClearDepthStencilValue(void)
			: ClearDepthStencilValue(0.0f, 0)
		{}

		/* Initializes a new instance of the clear depth/stencil object. */
		ClearDepthStencilValue(_In_ float depth, _In_ uint32 stencil)
			: Depth(depth), Stencil(stencil)
		{}
	};

	/* Defines how an attachment should be cleared. */
	union ClearValue
	{
		/* Specifies the color clear value. */
		ClearColorValue Color;
		/* Specifies the depth/stencil clear value. */
		ClearDepthStencilValue DepthStencil;
	};

	/* Defines the information required to start a render pass. */
	struct RenderPassBeginInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* The render pass to start. */
		RenderPassHndl RenderPass;
		/* The framebuffer that contains the attachments used in the render pass. */
		FramebufferHndl Framebuffer;
		/* The area that is affected by the render pass. */
		Rect2D RenderArea;
		/* The amount of value in the ClearValues field. */
		uint32 ClearValueCount;
		/* Specifies how each attachment should be cleared. */
		const ClearValue *ClearValues;

		/* Initializes an empty instance of a render pass begin info object. */
		RenderPassBeginInfo(void)
			: RenderPassBeginInfo(nullptr, nullptr, Rect2D())
		{}

		/* Initializes a new instance of a render pass begin info object. */
		RenderPassBeginInfo(_In_ RenderPassHndl renderPass, _In_ FramebufferHndl framebuffer, _In_ Rect2D renderArea)
			: Type(StructureType::RenderPassBeginInfo), Next(nullptr), RenderPass(renderPass),
			Framebuffer(framebuffer), RenderArea(renderArea), ClearValueCount(0), ClearValues(nullptr)
		{}
	};

	/* Defines the parameters of a label region. */
	struct DebugUtilsLabel
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies a null-terminate UTF-8 string that contains the name of the label. */
		const char *LabelName;
		/* Specifies an optional RGBA color value associated with the label. */
		float Color[4];

		/* Initializes an empty instance of a label region. */
		DebugUtilsLabel(void)
			: Type(StructureType::DebugUtilsLabelExt), Next(nullptr),
			LabelName(nullptr), Color{0.0f, 0.0f, 0.0f, 0.0f}
		{}
	};

	/* Defines the information needed to name an object. */
	struct DebugUtilsObjectNameInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the type of the object to be named. */
		ObjectType ObjectType;
		/* Specifies the handle to the object. */
		uint64 ObjectHandle;
		/* Specifies a null-terminated UTF-8 name to appply to the object. */
		const char *ObjectName;

		/* Initializes an empty instance of a object name object. */
		DebugUtilsObjectNameInfo(void)
			: DebugUtilsObjectNameInfo(ObjectType::Unknown, 0, nullptr)
		{}

		/* Initializes a new instance of a debug marker name info object. */
		DebugUtilsObjectNameInfo(_In_ Pu::ObjectType type, _In_ uint64 handle, _In_ const char *name)
			: Type(StructureType::DebugUtilsObjectNameInfoExt), Next(nullptr),
			ObjectType(type), ObjectHandle(handle), ObjectName(name)
		{}
	};

	/* Defines information to the debug utils messanger callback function. */
	struct DebugUtilsMessengerCallbackData
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the particular message ID. */
		const char *MessageIdName;
		/* Specifies the ID that triggered the message. */
		int32 MessageIdNumber;
		/* Specifies the details of the trigger conditions. */
		const char *Message;
		/* Specifies the amount of elements in the queue labels array. */
		uint32 QueueLabelCount;
		/* Specifies the active queues at the time the callback was triggered. */
		const DebugUtilsLabel *QueueLabels;
		/* Specifies the amount of elements in the command buffer labels array. */
		uint32 CmdBufLabelCount;
		/* Specifies the active command buffers at the time the callback was triggered. */
		const DebugUtilsLabel *CmdBufLabels;
		/* Specifies the amount of elements in the objects array. */
		uint32 ObjectCount;
		/* Specifies the objects related the the detected issue. */
		const DebugUtilsObjectNameInfo *Objects;

		/* Initializes an empty instance of the debug utils messanger callback data object. */
		DebugUtilsMessengerCallbackData(void)
			: Type(StructureType::DebugUtilsMessangerCallbackDataExt), Next(nullptr), Flags(0),
			MessageIdName(nullptr), MessageIdNumber(0), Message(nullptr), QueueLabelCount(0),
			QueueLabels(nullptr), CmdBufLabelCount(0), CmdBufLabels(nullptr), ObjectCount(0), Objects(nullptr)
		{}
	};

	/* Defines the information required to create a debug message callback. */
	struct DebugUtilsMessengerCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies which severity of event(s) will cause this callback to be invoked. */
		DebugUtilsMessageSeverityFlag MessageSeverity;
		/* Specifeis which type of event(s) will cause this callback to be invoked. */
		DebugUtilsMessageTypeFlag MessageType;
		/* Specifies the function to be called upon event trigger. */
		DebugUtilsMessengerCallback UserCallback;
		/* Specifies user data passed to the callback. */
		void *UserData;

		/* Initializes an empty instance of the debug utils messenger create info object. */
		DebugUtilsMessengerCreateInfo(void)
			: DebugUtilsMessengerCreateInfo(nullptr)
		{}

		/* Initializes a new instance of a debug utils messenger create info object. */
		DebugUtilsMessengerCreateInfo(_In_ DebugUtilsMessengerCallback callback)
			: Type(StructureType::DebugUtilsMessengerCreateInfoExt), Next(nullptr), Flags(0),
			MessageSeverity(DebugUtilsMessageSeverityFlag::Critical), MessageType(DebugUtilsMessageTypeFlag::All),
			UserCallback(callback), UserData(nullptr)
		{}
	};

	/* Defines the information required to create a new fence. */
	struct FenceCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the initial state and behavior of the fence. */
		FenceCreateFlag Flags;

		/* Initializes an empty instance of the fence create info object. */
		FenceCreateInfo(void)
			: FenceCreateInfo(FenceCreateFlag::None)
		{}

		/* Initializes a new instanace of the fence create info object. */
		FenceCreateInfo(_In_ FenceCreateFlag flags)
			: Type(StructureType::FenceCreateInfo), Next(nullptr), Flags(flags)
		{}
	};

	/* Defines the information required to create a memory buffer. */
	struct BufferCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies additional parameters for the buffer. */
		BufferCreateFlag Flags;
		/* Specifies the size (in bytes) of the buffer. */
		DeviceSize Size;
		/* Specifies the allowed usages of the buffer. */
		BufferUsageFlag Usage;
		/* Specifies how the buffer should be accessed by multiple queues. */
		SharingMode SharingMode;
		/* Specifies the number of elements of queue family indeces. */
		uint32 QueueFamilyIndexCount;
		/* Specifies the queue families that will access this buffer (only needed if sharing mode is Concurrent!). */
		const uint32 *QueueFamilyIndeces;

		/* Initializes an empty instance of the buffer create info object. */
		BufferCreateInfo(void)
			: BufferCreateInfo(0, BufferUsageFlag::Undefinfed)
		{}

		/* Initializes a new instance of the buffer create info object. */
		BufferCreateInfo(_In_ DeviceSize size, _In_ BufferUsageFlag usage)
			: Type(StructureType::BufferCreateInfo), Next(nullptr), Flags(BufferCreateFlag::None),
			Size(size), Usage(usage), SharingMode(SharingMode::Exclusive),
			QueueFamilyIndexCount(0), QueueFamilyIndeces(nullptr)
		{}

		/* Initializes a new instance of the buffer create info object. */
		BufferCreateInfo(_In_ DeviceSize size, _In_ BufferUsageFlag usage, _In_ const vector<uint32> &queueFamilies)
			: Type(StructureType::BufferCreateInfo), Next(nullptr), Flags(BufferCreateFlag::None),
			Size(size), Usage(usage), SharingMode(SharingMode::Concurrent),
			QueueFamilyIndexCount(static_cast<uint32>(queueFamilies.size())), QueueFamilyIndeces(queueFamilies.data())
		{}
	};

	/* Defines the requirement that need to be fulfilled for memory allocation. */
	struct MemoryRequirements
	{
	public:
		/* Specifies the size (in bytes) of the memory allocation. */
		DeviceSize Size;
		/* Specifies the allignment (in bytes) of the offset within the allocation. */
		DeviceSize Alignment;
		/* Specifies a bitmsask for every supported memory type for the resource. */
		uint32 MemoryTypeBits;

		/* Initializes an empty instance of the memory requirements object. */
		MemoryRequirements(void)
			: Size(0), Alignment(0), MemoryTypeBits(0)
		{}
	};

	/* Defines a unique memory type on a physical device. */
	struct MemoryType
	{
	public:
		/* Specifies the properties of the memory heap. */
		MemoryPropertyFlag PropertyFlags;
		/* Specifies which memory heap this memory type corresponds to. */
		uint32 HeapIndex;

		/* Initializes an empty instance of the memory type object. */
		MemoryType(void)
			: PropertyFlags(MemoryPropertyFlag::None), HeapIndex(0)
		{}
	};

	/* Defines a unique memory heap on a physical device. */
	struct MemoryHeap
	{
	public:
		/* Specifies the total size (in bytes) of the heap. */
		DeviceSize Size;
		/* Specifies the properties of the heap. */
		MemoryHeapFlag Flags;

		/* Initializes an empty insatnce of a memory heap object. */
		MemoryHeap(void)
			: Size(0), Flags(MemoryHeapFlag::None)
		{}
	};

	/* Defines the properties of a physical device's memory. */
	struct PhysicalDeviceMemoryProperties
	{
	public:
		/* Specifies the amount of memory types available. */
		uint32 MemoryTypeCount;
		/* Specifies all memory types that can be used to access memory allocated from the heaps. */
		MemoryType MemoryTypes[MaxMemoryTypes];
		/* Specifies the amount of memory heaps available. */
		uint32 MemoryHeapCount;
		/* Specifies all memory heaps available on the physical device. */
		MemoryHeap MemoryHeaps[MaxMemoryHeaps];

		/* Initializes an empty instance of the physical device memory properties object. */
		PhysicalDeviceMemoryProperties(void)
			: MemoryTypeCount(0), MemoryHeapCount(0)
		{}
	};

	/* Defines the information required to allocate memory on a physical device. */
	struct MemoryAllocateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the amount (in bytes) of the allocation. */
		DeviceSize AllocationSize;
		/* Specifies the index of the memory type (on the physical device) from which to make this allocation. */
		uint32 MemoryTypeIndex;

		/* Initializes an empty instance of the memory allocate info object. */
		MemoryAllocateInfo(void)
			: MemoryAllocateInfo(0, 0)
		{}

		/* Initializes a new instance of the memory allocate info object. */
		MemoryAllocateInfo(_In_ DeviceSize size, _In_ uint32 typeIdx)
			: Type(StructureType::MemoryAllocateInfo), Next(nullptr),
			AllocationSize(size), MemoryTypeIndex(typeIdx)
		{}
	};

	/* Defines information about a mapped memory change. */
	struct MappedMemoryRange
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the memory object of the range. */
		DeviceMemoryHndl Memory;
		/* Specifies a zero-based offset (in bytes) from the beginning of the memory offset. */
		DeviceSize Offset;
		/* Specifies either the size (in bytes) affecting the memory of WholeSize. */
		DeviceSize Size;

		/* Initializes an empty instance of a mapped memory range object. */
		MappedMemoryRange(void)
			: MappedMemoryRange(nullptr, 0, 0)
		{}

		/* Initializes a new instance of a mapped memory range object for the whole range. */
		MappedMemoryRange(_In_ DeviceMemoryHndl memory)
			: MappedMemoryRange(memory, 0, WholeSize)
		{}

		/* Initializes a new instance of a mapped memory range object. */
		MappedMemoryRange(_In_ DeviceMemoryHndl memory, _In_ DeviceSize offset, _In_ DeviceSize size)
			: Type(StructureType::MappedMemoryRange), Next(nullptr), 
			Memory(memory), Offset(offset), Size(size)
		{}
	};

	/* Defines the region of a buffer copy command. */
	struct BufferCopy
	{
	public:
		/* Specifies the offset (in bytes) from the start of the source buffer. */
		DeviceSize SrcOffset;
		/* Specifies the offset (in bytes) from the start of the destination buffer. */
		DeviceSize DstOffset;
		/* Specifies the number of bytes to copy. */
		DeviceSize Size;

		/* Initializes an empty instance of the buffer copy object. */
		BufferCopy(void)
			: SrcOffset(0), DstOffset(0), Size(0)
		{}

		/* Initializes a new instance of the buffer copy object. */
		BufferCopy(_In_ DeviceSize srcOffset, _In_ DeviceSize dstOffset, _In_ DeviceSize size)
			: SrcOffset(srcOffset), DstOffset(dstOffset), Size(size)
		{}
	};

	/* Defines the information required to create a new image. */
	struct ImageCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies aditional parameters of the image. */
		ImageCreateFlag Flags;
		/* Specifies the basic dimensionality of the image. */
		ImageType ImageType;
		/* Specifies the texel format for the image. */
		Format Format;
		/* Specifies the dimensions of the image. */
		Extent3D Extent;
		/* Specifies the number of levels of detail available for minified sampling of the image. */
		uint32 MipLevels;
		/* Specifies the number of layers in the image. */
		uint32 ArrayLayers;
		/* Specifies the number of samples per texel. */
		SampleCountFlag Samples;
		/* Specifies the tiling arrangement of the texel blocks in memory. */
		ImageTiling Tiling;
		/* Specifies the intended usage of the image. */
		ImageUsageFlag Usage;
		/* Specifies how the image behaves when accessed by multiple queue families. */
		SharingMode SharingMode;
		/* Specifies the amount of elements in the QueueFamilyIndeces array. */
		uint32 QueueFamilyIndexCount;
		/* Specifies the queue families that will access this image (when sharing mode is not Concurrent). */
		const uint32 *QueueFamilyIndeces;
		/* Specifies the initial layout of all image subresources of the image. */
		ImageLayout InitialLayout;

		/* Initializes an empty instance of the image create info object. */
		ImageCreateInfo(void)
			: ImageCreateInfo(ImageType::Image2D, Format::Undefined, Extent3D(), 0, 0, SampleCountFlag::None, ImageUsageFlag::None)
		{}

		/* Initializes a new instance of the image create info object. */
		ImageCreateInfo(_In_ Pu::ImageType type, _In_ Pu::Format format, _In_ Extent3D extent, _In_ uint32 mipLevels, _In_ uint32 arrayLayers, _In_ SampleCountFlag samples, _In_ ImageUsageFlag usage)
			: Type(StructureType::ImageCreateInfo), Next(nullptr), Flags(ImageCreateFlag::None),
			ImageType(type), Format(format), Extent(extent), MipLevels(mipLevels), 
			ArrayLayers(arrayLayers), Samples(samples), Tiling(ImageTiling::Optimal),
			Usage(usage), SharingMode(SharingMode::Exclusive), QueueFamilyIndexCount(0),
			QueueFamilyIndeces(nullptr), InitialLayout(ImageLayout::Undefined)
		{}
	};

	/* Defines a region of an image. */
	struct ImageSubresourceLayers
	{
	public:
		/* Specifies the color, depth and/or stencil aspects to be copied. */
		ImageAspectFlag AspectMask;
		/* Specifies the mipmap level to copy from. */
		uint32 MipLevel;
		/* Specifies the starting layer of layers to copy. */
		uint32 BaseArrayLayer;
		/* Specifies the amount of layers to copy. */
		uint32 LayerCount;

		/* Initializes a default instance of the image sub-resource layers object. */
		ImageSubresourceLayers(void)
			: AspectMask(ImageAspectFlag::Color), MipLevel(0),
			BaseArrayLayer(0), LayerCount(1)
		{}
	};

	/* Defines the region of an buffer to image copy command. */
	struct BufferImageCopy
	{
	public:
		/* Specifies the offset (in bytes) from the start of the buffer to where the image data is stored. */
		DeviceSize BufferOffset;
		/* Specifies an optional texel subregion of a larger two- or three-dimensional image. */
		uint32 BufferRowLength;
		/* Specifies an optional texel subregion of a larger two- or three-dimensional image. */
		uint32 BufferImageHeight;
		/* Specifies the image subresources of the image used for the copy command. */
		ImageSubresourceLayers ImageSubresource;
		/* Specifies the offset (in texels) of the sub-region of the source or destination image data. */
		Offset3D ImageOffset;
		/* Specifies the size (in texels) of the image to copy. */
		Extent3D ImageExtent;

		/* Initializes an empty istance of a buffer to image copy object. */
		BufferImageCopy(void)
			: BufferImageCopy(Extent3D())
		{}

		/* Initializes a new instance of a buffer to image copy object. */
		BufferImageCopy(_In_ Extent3D imageSize)
			: BufferOffset(0), BufferRowLength(0), BufferImageHeight(0),
			ImageSubresource(), ImageExtent(imageSize)
		{}
	};

	/* Defines the information required to create a new sampler. */
	struct SamplerCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the magnification filter to apply to lookups. */
		Filter MagFilter;
		/* Specifies the minification filter to apply to lookups. */
		Filter MinFilter;
		/* Specifies the mipmap filter to apply to lookups. */
		SamplerMipmapMode MipmapMode;
		/* Specifies the addressing mode for outside [0, 1] range for U coordinates. */
		SamplerAddressMode AddressModeU;
		/* Specifies the addressing mode for outside [0, 1] range for V coordinates. */
		SamplerAddressMode AddressModeV;
		/* Specifies the addressing mode for outside [0, 1] range for W coordinates. */
		SamplerAddressMode AddressModeW;
		/* Specifies the bais to be added to mipmap LoD calculations. */
		float MipLodBias;
		/* Specifies whether anisotropic filtering is enabled. */
		Bool32 AnisotropyEnable;
		/* Specifies the anisotropy value clamp used (if anisotrpic filtering is enabled). */
		float MaxAnisotropy;
		/* Specifies whether comparison against a reference value during lookups is enabled. */
		Bool32 CompareModeEnable;
		/* Specifies the compare operation used when comparing to a reference value. */
		CompareOp CompareOp;
		/* Specifies the minimum value used for clamping values produced by the LoD computation. */
		float MinLoD;
		/* Specifies the maximum value used for clamping values produced by the LoD computation. */
		float MaxLoD;
		/* Specifies the predefined border color to use. */
		BorderColor BorderColor;
		/* Specifies whether to use normalizes or unnormalizes texel coordinates to address texels of the images. */
		Bool32 UnnormalizedCoordinates;

		/* Initializes a default instance of the sampler create info object. */
		SamplerCreateInfo(void)
			: SamplerCreateInfo(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::Repeat)
		{}

		/* Initializes a new instance of the sampler create info object. */
		SamplerCreateInfo(_In_ Filter minMagFilter, _In_ SamplerMipmapMode mipmap, _In_ SamplerAddressMode addressMode)
			: Type(StructureType::SamplerCreateInfo), Next(nullptr), Flags(0),
			MagFilter(minMagFilter), MinFilter(minMagFilter), MipmapMode(mipmap),
			AddressModeU(addressMode), AddressModeV(addressMode), AddressModeW(addressMode),
			MipLodBias(0.0f), AnisotropyEnable(false), MaxAnisotropy(0.0f), CompareModeEnable(false),
			CompareOp(CompareOp::Always), MinLoD(0.0f), MaxLoD(0.0f), BorderColor(BorderColor::FloatTransparentBlack),
			UnnormalizedCoordinates(false)
		{}
	};

	/* Defines how many units of a specific descriptor type should be created. */
	struct DescriptorPoolSize
	{
	public:
		/* Specifies the type of descriptor. */
		DescriptorType Type;
		/* Specifies the amount of descriptors of that type of allocate. */
		uint32 DescriptorCount;

		/* Initializes an empty instance of a descriptor pool size object. */
		DescriptorPoolSize(void)
			: DescriptorPoolSize(DescriptorType::Sampler, 0)
		{}

		/* Initializes a new instance of a descriptor pool size object. */
		DescriptorPoolSize(_In_ DescriptorType type, _In_ uint32 count)
			: Type(type), DescriptorCount(count)
		{}
	};

	/* Defines the information required to create a new descriptor pool. */
	struct DescriptorPoolCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies additional parameter for a descriptor pool. */
		DescriptorPoolCreateFlag Flags;
		/* Specifies the maximum amount of descriptor sets that can be allocated from the pool. */
		uint32 MaxSets;
		/* Specifies the amount of elements in the PoolSize field. */
		uint32 PoolSizeCount;
		/* Specifies how many descriptors from each type can be created via the pool. */
		const DescriptorPoolSize *PoolSizes;
		
		/* Initializes an empty instance of the descriptor pool create info object. */
		DescriptorPoolCreateInfo(void)
			: Type(StructureType::DescriptorPoolCreateInfo), Next(nullptr), Flags(DescriptorPoolCreateFlag::None),
			MaxSets(0), PoolSizeCount(0), PoolSizes(nullptr)
		{}

		/* Initializes a new instance of the descriptor pool create info object. */
		DescriptorPoolCreateInfo(_In_ uint32 maxSets, _In_ const vector<DescriptorPoolSize> &sizes)
			: Type(StructureType::DescriptorPoolCreateInfo), Next(nullptr), Flags(DescriptorPoolCreateFlag::FreeDescriptorSet),
			MaxSets(maxSets), PoolSizeCount(static_cast<uint32>(sizes.size())), PoolSizes(sizes.data())
		{}
	};

	/* Defines a layout for a descriptor set. */
	struct DescriptorSetLayoutBinding
	{
	public:
		/* Specifies the binding number of this entry. */
		uint32 Binding;
		/* Specifies the type of resource descriptors used for this binding. */
		DescriptorType DescriptorType;
		/* Specifies the amount of descriptors contained in the binding. */
		uint32 DescriptorCount;
		/* Specifies the parts of the graphics pipeline that can access a resource for this binding. */
		ShaderStageFlag StageFlags;
		/* Specifies an optional set of constant samplers that cannot be changed later. */
		const SamplerHndl *ImmutableSamplers;
		
		/* Initializes an empty instance of the descriptor set layout binding object. */
		DescriptorSetLayoutBinding(void)
			: DescriptorSetLayoutBinding(0, DescriptorType::Sampler, 0)
		{}

		/* Initializes a new instance of the descriptor set layout binding object. */
		DescriptorSetLayoutBinding(_In_ uint32 binding, _In_ Pu::DescriptorType type, _In_ uint32 count)
			: Binding(binding), DescriptorType(type), DescriptorCount(count),
			StageFlags(ShaderStageFlag::All), ImmutableSamplers(nullptr)
		{}
	};

	/* Defines the information required to create a new descriptor set layout. */
	struct DescriptorSetLayoutCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies additions parameters for the descriptor set layout. */
		DescriptorSetLayoutCreateFlags Flags;
		/* Specifies the amount of elements in the Bindings field. */
		uint32 BindingCount;
		/* Specifies all bindings associated with this descriptor set layout. */
		const DescriptorSetLayoutBinding *Bindings;

		/* Initializes an empty instance of the descriptor set layout create info object. */
		DescriptorSetLayoutCreateInfo(void)
			: Type(StructureType::DescriptorSetLayourCreateInfo), Next(nullptr), Flags(DescriptorSetLayoutCreateFlags::None),
			BindingCount(0), Bindings(nullptr)
		{}

		/* Initializes a new instance of the descriptor set layout create info object. */
		DescriptorSetLayoutCreateInfo(_In_ const vector<DescriptorSetLayoutBinding> &bindings)
			: Type(StructureType::DescriptorSetLayourCreateInfo), Next(nullptr), Flags(DescriptorSetLayoutCreateFlags::None),
			BindingCount(static_cast<uint32>(bindings.size())), Bindings(bindings.data())
		{}
	};

	/* Defines the information required to allocate a new descriptor set. */
	struct DescriptorSetAllocateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the pool from which to allocate the set(s). */
		DescriptorPoolHndl DescriptorPool;
		/* Specifies how many descriptor sets to be allocated from the pool. */
		uint32 DescriptorSetCount;
		/* Specifies how the corresponding descriptor is allocated. */
		const DescriptorSetLayoutHndl *SetLayouts;

		/* Initializes an empty instance of the descriptor set allocate info object. */
		DescriptorSetAllocateInfo(void)
			: Type(StructureType::DescriptorSetAllocateInfo), Next(nullptr), 
			DescriptorPool(nullptr), DescriptorSetCount(0), SetLayouts(nullptr)
		{}

		/* Initializes a new instance of the descriptor set allocate info object. */
		DescriptorSetAllocateInfo(_In_ DescriptorPoolHndl pool, _In_ DescriptorSetLayoutHndl layout)
			: Type(StructureType::DescriptorSetAllocateInfo), Next(nullptr), DescriptorPool(pool),
			DescriptorSetCount(1), SetLayouts(&layout)
		{}
	};

	/* Defines the information needed for an image descriptor. */
	struct DescriptorImageInfo
	{
	public:
		/* Specifies the sampler the image uses. */
		SamplerHndl Sampler;
		/* Specifies the image view for the image. */
		ImageViewHndl ImageView;
		/* Specifies how the image should be used. */
		ImageLayout ImageLayout;

		/* Initializes an empty instance of a descriptor image info object. */
		DescriptorImageInfo(void)
			: Sampler(nullptr), ImageView(nullptr), ImageLayout(ImageLayout::Undefined)
		{}

		/* Initializes a new instance of a descriptor image info object. */
		DescriptorImageInfo(_In_ SamplerHndl sampler, _In_ ImageViewHndl view)
			: Sampler(sampler), ImageView(view), ImageLayout(ImageLayout::ShaderReadOnlyOptimal)
		{}
	};

	/* Defines the information needed for a buffer descriptor. */
	struct DescriptorBufferInfo
	{
	public:
		/* Specifies the buffer resource. */
		BufferHndl Buffer;
		/* Specifies the offset (in bytes) from where to start. */
		DeviceSize Offset;
		/* Specifies the size (in bytes) that is used for this descriptor. */
		DeviceSize Range;

		/* Initializes an empty instance of a descriptor buffer info object. */
		DescriptorBufferInfo(void)
			: Buffer(nullptr), Offset(0), Range(0)
		{}

		/* Initializes a new instance of a descriptor buffer info object. */
		DescriptorBufferInfo(_In_ BufferHndl buffer, _In_ DeviceSize offset, _In_ DeviceSize range)
			: Buffer(buffer), Offset(offset), Range(range)
		{}
	};

	/* Defines the parameters of a descriptor set write operation. */
	struct WriteDescriptorSet
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the descriptor set to update. */
		DescriptorSetHndl DstSet;
		/* Specifies the descriptor bindings within the set to update. */
		uint32 DstBinding;
		/* Specifies the starting element of the array to update. */
		uint32 DstArrayElement;
		/* Specifies the number of descriptors to update. */
		uint32 DescriptorCount;
		/* Specifies the type of each descriptor to update. */
		DescriptorType DescriptorType;
		/* Specifies the descriptor information required for image descriptors. */
		const DescriptorImageInfo *ImageInfo;
		/* Specifies the descriptor information required for buffer descriptors. */
		const DescriptorBufferInfo *BufferInfo;
		/* Specifies the descriptor information required for texel buffer descriptors. */
		const BufferViewHndl *TexelBufferView;

		/* Initializes an empty instance of a descriptor set write operation parameters object. */
		WriteDescriptorSet(void)
			: Type(StructureType::WriteDescriptorSet), Next(nullptr), DstSet(nullptr), DstBinding(0),
			DstArrayElement(0), DescriptorType(DescriptorType::UniformBuffer), ImageInfo(nullptr),
			DescriptorCount(0), BufferInfo(nullptr), TexelBufferView(nullptr)
		{}

		/* Initializes a new instance of a descriptor set write operation parameters object as an buffer write operation. */
		WriteDescriptorSet(_In_ DescriptorSetHndl set, _In_ uint32 binding, _In_ const DescriptorBufferInfo &info)
			: Type(StructureType::WriteDescriptorSet), Next(nullptr), DstSet(set), DstBinding(binding),
			DstArrayElement(0), DescriptorType(DescriptorType::UniformBuffer), ImageInfo(nullptr),
			DescriptorCount(1), BufferInfo(&info), TexelBufferView(nullptr)
		{}

		/* Initializes a new instance of a descriptor set write operation parameters object as an buffer write operation. */
		WriteDescriptorSet(_In_ DescriptorSetHndl set, _In_ uint32 binding, _In_ const vector<DescriptorBufferInfo> &info)
			: Type(StructureType::WriteDescriptorSet), Next(nullptr), DstSet(set), DstBinding(binding),
			DstArrayElement(0), DescriptorType(DescriptorType::UniformBuffer), ImageInfo(nullptr),
			DescriptorCount(static_cast<uint32>(info.size())), BufferInfo(info.data()), TexelBufferView(nullptr)
		{}

		/* Initializes a new instance of a descriptor set write operation parameters object as an image write operation. */
		WriteDescriptorSet(_In_ DescriptorSetHndl set, _In_ uint32 binding, _In_ const DescriptorImageInfo &info)
			: Type(StructureType::WriteDescriptorSet), Next(nullptr), DstSet(set), DstBinding(binding),
			DstArrayElement(0), DescriptorType(DescriptorType::CombinedImageSampler), BufferInfo(nullptr),
			DescriptorCount(1), ImageInfo(&info), TexelBufferView(nullptr)
		{}

		/* Initializes a new instance of a descriptor set write operation parameters object as an image write operation. */
		WriteDescriptorSet(_In_ DescriptorSetHndl set, _In_ uint32 binding, _In_ const vector<DescriptorImageInfo> &info)
			: Type(StructureType::WriteDescriptorSet), Next(nullptr), DstSet(set), DstBinding(binding),
			DstArrayElement(0), DescriptorType(DescriptorType::CombinedImageSampler), BufferInfo(nullptr),
			DescriptorCount(static_cast<uint32>(info.size())), ImageInfo(info.data()), TexelBufferView(nullptr)
		{}
	};

	/* Defines the parameters of a descriptor set copy operation. */
	struct CopyDescriptorSet
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Specifies the set used as the source for the copy operation. */
		DescriptorSetHndl SrcSet;
		/* Specifies the binding in the source set used as the source for the copy operation. */
		uint32 SrcBinding;
		/* Specifies the starting array element in the source set. */
		uint32 SrcArrayElement;
		/* Specifies the set used as the destination for the copy operation. */
		DescriptorSetHndl DstSet;
		/* Specifies the binding in the destination set used as the source for the copy operation. */
		uint32 DstBinding;
		/* Specfies the starting array element in the destination set. */
		uint32 DstArrayElement;
		/* Specifies the amount of descriptors to copy from the source to the destination. */
		uint32 DescriptorCount;

		/* Initializes an empty instance of the copy descriptor operation parameter object. */
		CopyDescriptorSet(void)
			: Type(StructureType::CopyDescriptorSet), Next(nullptr), DescriptorCount(0),
			SrcSet(nullptr), SrcBinding(0), SrcArrayElement(0),
			DstSet(nullptr), DstBinding(0), DstArrayElement(0)
		{}
	};

	/* Defines the information needed to create a new query pool. */
	struct QueryPoolCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* The type of queries managed by the pool. */
		QueryType QueryType;
		/* The number of queries managed by the pool. */
		uint32 QueryCount;
		/* Specifies which counters will be returned in queries on the new pool. */
		QueryPipelineStatisticFlag PipelineStatistics;

		/* Initializes an empty instance of a query pool create info object. */
		QueryPoolCreateInfo(void)
			: QueryPoolCreateInfo(QueryType::Timestamp, 0)
		{}

		/* Initializes a new instance of a query pool create info object. */
		QueryPoolCreateInfo(_In_ Pu::QueryType type, _In_ uint32 count)
			: QueryPoolCreateInfo(type, count, QueryPipelineStatisticFlag::None)
		{}

		/* Initializes a new instance of a query pool create info object. */
		QueryPoolCreateInfo(_In_ Pu::QueryType type, _In_ uint32 count, _In_ QueryPipelineStatisticFlag stats)
			: Type(StructureType::QueryPoolCreatInfo), Next(nullptr), Flags(0),
			QueryType(type), QueryCount(count), PipelineStatistics(stats)
		{}
	};

	/* Defines the information needed to create a new pipeline cache. */
	struct PipelineCacheCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* Specifies the size (in bytes) of the InitialData memory. */
		size_t InitialDataSize;
		/* Specifies a pointer to the previously retrieved pipeline cache data store. */
		const void *InitialData;

		/* Initializes an empty instance of a pipeline cache create info object. */
		PipelineCacheCreateInfo(void)
			: PipelineCacheCreateInfo(0, nullptr)
		{}

		/* Initializes a new instance of a pipeline cache create info object. */
		PipelineCacheCreateInfo(_In_ size_t size, _In_ const void *data)
			: Type(StructureType::PipelineCacheCreateInfo), Next(nullptr), Flags(0),
			InitialDataSize(size), InitialData(data)
		{}
	};

	/* Defines the region of an image blit operation. */
	struct ImageBlit
	{
	public:
		/* The subresource to blit from. */
		ImageSubresourceLayers SrcSubresource;
		/* Specifies a box region of the source region within the subresource. */
		Offset3D SrcOffsets[2];
		/* The subresource to blit into. */
		ImageSubresourceLayers DstSubresource;
		/* Specifies a box region of the destination region within the subresource. */
		Offset3D DstOffsets[2];

		/* Initializes an empty instance of an image blit object. */
		ImageBlit(void)
			: SrcOffsets{ { 0, 0, 0 }, { 0, 0, 0 } }, DstOffsets{ { 0, 0, 0 }, { 0, 0, 0 } }
		{}

		/* Initializes a new instance of an image blit object for a blit between the full resources of two images. */
		ImageBlit(_In_ uint32 layer, _In_ uint32 srcMip, _In_ uint32 dstMip, _In_ Extent2D size)
		{
			SrcSubresource.BaseArrayLayer = layer;
			DstSubresource.BaseArrayLayer = layer;

			SrcSubresource.MipLevel = srcMip;
			DstSubresource.MipLevel = dstMip;

			SrcOffsets[1].X = static_cast<int32>(size.Width >> srcMip);
			SrcOffsets[1].Y = static_cast<int32>(size.Height >> srcMip);
			SrcOffsets[1].Z = 1;

			DstOffsets[1].X = static_cast<int32>(size.Width >> dstMip);
			DstOffsets[1].Y = static_cast<int32>(size.Height >> dstMip);
			DstOffsets[1].Z = 1;
		}
	};

#ifdef _WIN32
	/* Defines the information required to create a surface on the Windows platform. */
	struct Win32SurfaceCreateInfo
	{
	public:
		/* The type of this structure. */
		const StructureType Type;
		/* Pointer to an extension-specific structure or nullptr. */
		const void *Next;
		/* Reserved. */
		Flags Flags;
		/* The handle to the instance. */
		HINSTANCE Instance;
		/* The handle to the window. */
		HWND Window;

		/* Initializes a default instance of the surface create info object. */
		Win32SurfaceCreateInfo(void)
			: Win32SurfaceCreateInfo(nullptr, nullptr)
		{}

		/* Initializes a new instance of te surface create info object. */
		Win32SurfaceCreateInfo(_In_ HINSTANCE hinstance, _In_ HWND hwnd)
			: Type(StructureType::Win32SurfaceCreateInfoKhr), Next(nullptr),
			Flags(0), Instance(hinstance), Window(hwnd)
		{}
	};
#endif
}