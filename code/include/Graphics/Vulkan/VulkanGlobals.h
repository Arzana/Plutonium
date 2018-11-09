#pragma once
#include "Core/Math/Constants.h"
#include "VulkanPlatform.h"

/* Defines a Vulkan object handle. */
#define VK_DEFINE_HANDLE(hndl)						using hndl##Hndl = void*

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
/* Defines a non dispatchable handle. */
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)	using object##Hndl = void*
#else
/* Defines a non dispatchable handle. */
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)	using object##Hndl = uint64_t
#endif

namespace Pu
{
	/* Creates a version number with the specified major, minor and patch versions. */
	_Check_return_ constexpr inline uint32 makeVersion(_In_ uint32 major, _In_ uint32 minor, _In_ uint32 patch)
	{
		return (major << 22) | (minor << 12) | (patch);
	}

	/* Gets the major version from a packed version. */
	_Check_return_ constexpr inline uint32 getMajor(_In_ uint32 version)
	{
		return version >> 22;
	}

	/* Gets the minor version from a packed version. */
	_Check_return_ constexpr inline uint32 getMinor(_In_ uint32 version)
	{
		return (version >> 12) & 0x3FF;
	}

	/* Gets the patch version from a packed version. */
	_Check_return_ constexpr inline uint32 getPatch(_In_ uint32 version)
	{
		return version & 0xFFF;
	}

	/* Defines the current Vulkan API version. */
	constexpr uint32 VulkanVersion = makeVersion(1, 0, 3);

	/* Defines the type of generic flags. */
	using Flags = uint32;
	/* Defines the type of a device size. */
	using DeviceSize = uint64;
	/* Defines the type of a sample mask. */
	using SampleMask = uint32;
	/* Defines a 32 bit boolean value. */
	using Bool32 = uint32;

	/* Defines a handle to a Vulkan state. */
	VK_DEFINE_HANDLE(Instance);
	/* Defines a handle to a physical device. */
	VK_DEFINE_HANDLE(PhysicalDevice);
	/* Defines the handle to a device. */
	VK_DEFINE_HANDLE(Device);
	/* Defines the handle to a Vulkan queue. */
	VK_DEFINE_HANDLE(Queue);
	/* Defines the handle to a semaphore. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Semaphore);
	/* Defines the handle to a command buffer. */
	VK_DEFINE_HANDLE(CommandBuffer);
	/* Defines the handle to a fence. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Fence);
	/* Defines the handle to a device memory object. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(DeviceMemory);
	/* Defines the handle to a GPU buffer. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Buffer);
	/* Defines the handle to a multidimensional array. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Image);
	/* Defines the handle to an event. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Event);
	/* Defines the handle to a query manager. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(QueryPool);
	/* Defines the handle to a buffer view object. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(BufferView);
	/* Defines the handle to a image view object. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(ImageView);
	/* Defines the handle to a shader module. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(ShaderModule);
	/* Defines the handle to a pipeline cache object. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(PipelineCache);
	/* Defines the handle to a pipeline layout descriptor. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(PipelineLayout);
	/* Defines the handle to a render pass. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(RenderPass);
	/* Defines the handle to a compute or graphics pipeline. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Pipeline);
	/* Defines the handle to a descriptor set layout. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(DescriptorSetLayout);
	/* Defines the handle to an image sampler. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Sampler);
	/* Defines the handle to a descriptor manager. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(DescriptorPool);
	/* Defines the handle to a descriptor set. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(DescriptorSet);
	/* Defines the handle to a frame buffer. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Framebuffer);
	/* Defines the handle to a command manager. */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(CommandPool);
	/* Defines the handle to a surface (extension). */
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(Surface);

	/* Defines the clamp value for level of detail none. */
	constexpr float LoDClampNone = 1000.0f;
	/* Used to set the mip maps to be used to all. */
	constexpr uint32 RemainingMipLevels = ~0U;
	/* Used to set the array layers to be used to all. */
	constexpr uint32 RemainingArrayLayers = ~0U;
	/* Used to set the memory range to the whole range. */
	constexpr DeviceSize WholeSize = ~0ULL;
	// TODO: add AttachmentUnused to AttachmentRefrence.
	/*  Used to indicate that the queue family should be ignored. */
	constexpr uint32 QueueFamilyIgnored = ~0U;
	/* Used to indicate non clamped subpass scope */
	constexpr uint32 SubpassExternal = ~0U;
	/* The maximum name length of a physical device name. */
	constexpr size_t MaxPhysicalDeviceNameSize = 256;
	/* The amount of universally unique indentifiers for devices. */
	constexpr size_t UUIDSize = 16;
	/* The maximum amount of physical device memory types. */
	constexpr size_t MaxMemoryTypes = 32;
	/* The maximum amount of physical device memory heaps. */
	constexpr size_t MaxMemoryHeaps = 16;
	/* The maximum name length of an layer extension. */
	constexpr size_t MaxExtensionNameSize = 256;
	/* The maximum string length of a layer description. */
	constexpr size_t MaxDescriptionSize = 256;
}