#pragma once
#include "VulkanFunctions.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Pu
{
	/* Defines application info. */
	struct ApplicationInfo
	{
	public:
		/* The type of this structure. */
		StructureType Type;
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
		StructureType Type;
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
			: InstanceCreateInfo(nullptr)
		{}

		/* Initializes a new instance of a instance create information object. */
		InstanceCreateInfo(_In_ const Pu::ApplicationInfo *applicationInfo)
			: Type(StructureType::InstanceCreateInfo), Next(nullptr),
			Flags(0), ApplicationInfo(applicationInfo),
			EnabledLayerCount(0), EnabledLayerNames(nullptr),
			EnabledExtensionCount(0), EnabledExtensionNames(nullptr)
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
		Bool32 RobustBufferAccess : 32;
		/* Specifies the full 32-bit range of indices is supported for indexed draw calls. */
		Bool32 FullDrawIndexUint32 : 32;
		/* Specifies whether image views with a ViewType of CubeArray can be created. */
		Bool32 ImageCubeArray : 32;
		/* Specifies whether the PipelineColorBlendAttachmentState settgins are controlled independently per-attachment. */
		Bool32 IndependentBlend : 32;
		/* Specifies whether geometry shaders are supported. */
		Bool32 GeometryShader : 32;
		/* Specifies whether trssellation control and evaluation shaders are supported. */
		Bool32 TessellationShader : 32;
		/* Specifies whether Sample Shading and multisample interpolation are supported. */
		Bool32 SampleRateShading : 32;
		/* Specifies whether blend operations which take two sources are supported. */
		Bool32 DualSrcBlend : 32;
		/* Specifies whether logic operatrions are supported. */
		Bool32 LogicOp : 32;
		/* Specifies whether multiple draw indirect is supported. */
		Bool32 MultiDrawIndirect : 32;
		/* Specifies whether indirect draw calls support FirstInstance parameters. */
		Bool32 DrawIndirectFirstInstance : 32;
		/* Specifies whether depth clamping is supported. */
		Bool32 DepthClamp : 32;
		/* Specifies whether depth bias clamping is supported. */
		Bool32 DepthBiasClamp : 32;
		/* Specifies whether point and wireframe fill modes are supported. */
		Bool32 FillModeNonSolid : 32;
		/* Specifies whether depth bounds tests are supported. */
		Bool32 DepthBounds : 32;
		/* Specifies whether lines with width other than 1.0 are supported. */
		Bool32 WideLines : 32;
		/* Specifies whether points with sie greater than 1.0f are supported. */
		Bool32 LargePoints : 32;
		/* Specifies whether the implementation is able to replace the alpha value of color fragment output from the fragment shader. */
		Bool32 AlphaToOne : 32;
		/* Specifies whether more than one viewport is supported. */
		Bool32 MultiViewport : 32;
		/* Specifies whether anisotropic filtering is supported. */
		Bool32 SamplerAnisotropy : 32;
		/* Specifies whether all of the ETC2 and EAC compressed texture formats are supported. */
		Bool32 TextureCompressionETC2 : 32;
		/* Specifies whether all of the ASTC_LDR compressed texture formats are supported. */
		Bool32 TextureCompressionASTC_LDR : 32;
		/* Specifies whether all of the BC compressed texture formats are supported. */
		Bool32 TextureCompressionBC : 32;
		/* Specifies whether occlusion queries returning actual sample counts are supported. */
		Bool32 OcclusionQueryPrecise : 32;
		/* Specifies whether the pipeline staticstics queries are supported. */
		Bool32 PipelineStatisticsQuery : 32;
		/* Specifies whether storage buffers and images support stores and atomic operations in the vertex, tessellation and geometry shader stages. */
		Bool32 VertexPipelineStoresAndAtomics : 32;
		/* Specifies whether storage buffers and images support stores and atomic operations in the fragment shader stage. */
		Bool32 FragmentStoresAndAtomics : 32;
		/* Specifies whether the PointSize built-in decoration is available in the tessellation control, tessellation evaluation and geometry shader stages. */
		Bool32 ShaderTessellationAndGeometryPointSize : 32;
		/* Specifies whether the extended set of images gather instructions are available in shader code. */
		Bool32 ShaderImageGatherExtended : 32;
		/* Specifies whether all the extended storage image formats are available in shader code. */
		Bool32 ShaderStorageImageExtendedFormats : 32;
		/* Specifies whether multisampled storage images are supported. */
		Bool32 ShaderStorageImageMultisample : 32;
		/* Specifies whether storage images require a format qualifier to be specified when reading from storage images. */
		Bool32 ShaderStorageImageReadWithoutFormat : 32;
		/* Specifies whether storage images require a format qualifier to be specified when writing to storage images. */
		Bool32 ShaderStorageImageWriteWithoutFormat : 32;
		/* Specifies whether arrays of uniform buffers can be indexed by dynamically uniform integer expressions in shader code. */
		Bool32 ShaderUniformBufferArrayDynamicIndexing : 32;
		/* Specifies whether ararys of samplers or sampled images can be indexed by dynamically uniform integer expressions in shader code. */
		Bool32 ShaderSampledImageArrayDynamicIndexing : 32;
		/* Specifies whether arrays of storage buffers can be indexed by dynamically uniform integer expressions in shader code. */
		Bool32 ShaderStorageBufferArrayDynamicIndexing : 32;
		/* Specifies whether arrays of storage images can be indexed by dynamically uniform interger expressions in shader code. */
		Bool32 ShaderStorageImageArrayDynamicIndexing : 32;
		/* Specifies whether clip distances are supported in shader code. */
		Bool32 ShaderClipDistance : 32;
		/* Specifies whether cull distances are supported in shader code. */
		Bool32 ShaderCullDistance : 32;
		/* Specifies whether doubles are supported in shader code. */
		Bool32 ShaderFloat64 : 32;
		/* Specifies whether longs are supported in shader code. */
		Bool32 ShaderInt64 : 32;
		/* Defines whether shorts are supported in shader code. */
		Bool32 ShaderInt16 : 32;
		/* Specifies whether image operations that return residency information are supported in shader code. */
		Bool32 ShaderResourceResidency : 32;
		/* Specifies whether image operations that specify the minimum resource LOD are supported in shader code. */
		Bool32 ShaderResourceMinLod : 32;
		/* Specifies whether resource memory can be managed at opaque sparse block level instead of at the object level. */
		Bool32 SparseBinding : 32;
		/* Specifies whether the device can access partially resident buffers. */
		Bool32 SparseResidencyBuffer : 32;
		/* Specifies whether the deivce can access partially resident 2D images with 1 sample per pixel. */
		Bool32 SparseResidencyImage2D : 32;
		/* Specifies whether the device can access partially resident 3D images. */
		Bool32 SparseResidencyImage3D : 32;
		/* Specifies whether the physical device can access partially resident 2D images with 2 samples per pixel. */
		Bool32 SparseResidency2Samples : 32;
		/* Specifies whether the physical device can access partially resident 2D images with 4 samples per pixel. */
		Bool32 SparseResidency4Samples : 32;
		/* Specifies whether the physical device can access partially resident 2D images with 8 samples per pixel. */
		Bool32 SparseResidency8Samples : 32;
		/* Specifies whether the physical device can access partially resident 2D images with 16 samples per pixel. */
		Bool32 SparseResidency16Samples : 32;
		/* specifies whether the physical device can correctly access data aliased into multiple locations. */
		Bool32 SparseResidencyAliased : 32;
		/* Specifies whether all pipelines that will be bound to a command buffer during a subpass with no attachments must have the same value for PipelineMultisampleStateCreateInfo::RasterizationSamples. */
		Bool32 VariableMultisampleRate : 32;
		/* Specifies whether a secondary command buffer may be executed while a query is active. */
		Bool32 InheritedQueries : 32;

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
		StructureType Type;
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
		StructureType Type;
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
		DeviceCreateInfo(_In_ uint32 queueCreateInfoCount, _In_ DeviceQueueCreateInfo *queueCreateInfos, 
			_In_opt_ uint32 enabledExtensionCount = 0, _In_opt_ const char *const *enabledExtensionNames = nullptr, 
			_In_opt_ const PhysicalDeviceFeatures *enabledFeatures = nullptr)
			: Type (StructureType::DeviceCreatInfo), Next(nullptr), Flags(0),
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
	};

#ifdef _WIN32
	/* Defines the information required to create a surface on the Windows platform. */
	struct Win32SurfaceCreateInfo
	{
	public:
		/* The type of this structure. */
		StructureType Type;
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