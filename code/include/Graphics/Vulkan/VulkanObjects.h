#pragma once
#include "Core/Platform/Windows/Windows.h"
#include "VulkanFunctions.h"

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
			Surface(surface), MinImageCount(2), ImageFormat(Format::Undefined),
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
			: CommandPoolCreateInfo(0)
		{}

		/* Initializes a new instance of the command pool create info object. */
		CommandPoolCreateInfo(_In_ uint32 queueFamilyIndex)
			: Type(StructureType::CommandPoolCreateInfo), Next(nullptr),
			Flags(CommandPoolCreateFlag::None), QueueFamilyIndex(queueFamilyIndex)
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
			Flags(CommandBufferUsageFlag::SimultaneousUse), InheritanceInfo(nullptr)
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
			: ImageSubresourceRange(ImageAspectFlag::None)
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
			SrcQueueFamilyIndex(0), DstQueueFamilyIndex(0), Buffer(buffer),
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
		/* The index of the attachment of the render pass. */
		uint32 Attachment;
		/* Specifies the layout the attachment uses during the subpass. */
		ImageLayout Layout;

		/* Initializes an empty instance of a attachment refrence. */
		AttachmentReference(void)
			: AttachmentReference(0, ImageLayout::Undefined)
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
		/* Specifies which of the render pass's attachments can be read in the fragment shader stage. */
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
	};

	/* Defines a subpass dependency. */
	struct SubpassDependency
	{
	public:
		/* Specifies the index of the first subpass in the dependency. */
		uint32 SrcSubpass;
		/* Specifies the index of the seconds subpass in the dependency. */
		uint32 DstSubpass;
		/* Specifies the source stage mask. */
		PipelineStageFlag SrcStageMask;
		/* Specifies the destination stage mask. */
		PipelineStageFlag DstStageMask;
		/* Specifies the source access. */
		AccessFlag SrcAccessMask;
		/* Specifies the destination access. */
		AccessFlag DstAcccessMask;
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
			SrcAccessMask(AccessFlag::None), DstAcccessMask(AccessFlag::None), DependencyFlags(DependencyFlag::None)
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

		/* Initializes a new instance of the render pass create info object. */
		RenderPassCreateInfo(void)
			: Type(StructureType::RenderPassCreateInfo), Next(nullptr), Flags(0),
			AttachmentCount(0), Attachments(nullptr), SubpassCount(0), Subpasses(nullptr),
			DependencyCount(0), Dependencies(nullptr)
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
			: ShaderModuleCreateInfo("")
		{}

		/* Initializes a new instance of the shader module create info object. */
		ShaderModuleCreateInfo(_In_ string code)
			: Type(StructureType::ShaderModuleCreateInfo), Next(nullptr), Flags(0),
			CodeSize(code.length()), Code(reinterpret_cast<const uint32*>(code.data()))
		{}
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