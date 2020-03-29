#pragma once
#include <map>
#include "Queue.h"
#include "VulkanProcedres.h"
#include "Core/Collections/Vector.h"

namespace Pu
{
	class PhysicalDevice;

	/* Defines a Vulkan logical device. */
	class LogicalDevice
	{
	public:
		LogicalDevice(_In_ const LogicalDevice&) = delete;
		/* Move constructor. */
		LogicalDevice(_In_ LogicalDevice &&value);
		/* Releases the logical device. */
		~LogicalDevice(void)
		{
			Destory();
		}

		_Check_return_ LogicalDevice& operator =(_In_ const LogicalDevice&) = delete;
		/* Move assignment. */
		_Check_return_ LogicalDevice& operator =(_In_ LogicalDevice &&other);

		/* Gets the specific graphics queue created with the logical device. */
		_Check_return_ inline Queue& GetGraphicsQueue(_In_ uint32 queueIndex)
		{
			return GetQueue(graphicsQueueFamily, queueIndex);
		}

		/* Gets the specific transfer queue created with the logical device. */
		_Check_return_ inline Queue& GetTransferQueue(_In_ uint32 queueIndex)
		{
			return GetQueue(transferQueueFamily, queueIndex);
		}

		/* Gets the specific queue created with the logical device. */
		_Check_return_ inline Queue& GetQueue(_In_ uint32 familyIndex, _In_ uint32 queueIndex)
		{
			return queues.at(familyIndex).at(queueIndex);
		}

		/* Gets the family index of the default graphics queues. */
		_Check_return_ inline uint32 GetGraphicsQueueFamily(void) const
		{
			return graphicsQueueFamily;
		}

		/* Gets the family index of the default transfer queues. */
		_Check_return_ inline uint32 GetTransferQueueFamily(void) const
		{
			return transferQueueFamily;
		}

		/* Gets the physical device the logical device was created on. */
		_Check_return_ inline const PhysicalDevice& GetPhysicalDevice(void) const
		{
			return *parent;
		}

		/* Gets the physical device the logical device was created on. */
		_Check_return_ inline PhysicalDevice& GetPhysicalDevice(void)
		{
			return *parent;
		}

		/* Halts the current thread until the logical device has reached an idle state, this should only be used when no other method of synchronization is possible! */
		void WaitIdle(void) const
		{
			VK_VALIDATE(vkDeviceWaitIdle(hndl), PFN_vkDeviceWaitIdle);
		}

		/* Gets whether a specific device extension is enabled. */
		_Check_return_ inline bool IsExtensionEnabled(_In_ const char *extension)
		{
			return enabledExtensions.contains(extension);
		}

		/* Sets the family index of the graphics and transfer queues. */
		void SetQueues(_In_ uint32 graphics, _In_ uint32 transfer);

	private:
		friend class Application;
		friend class PhysicalDevice;
		friend class Semaphore;
		friend class Swapchain;
		friend class CommandPool;
		friend class Queue;
		friend class CommandBuffer;
		friend class GameWindow;
		friend class Shader;
		friend class Renderpass;
		friend class ImageView;
		friend class Framebuffer;
		friend class GraphicsPipeline;
		friend class Fence;
		friend class Buffer;
		friend class Image;
		friend class DescriptorPool;
		friend class DescriptorSetBase;
		friend class DescriptorSetLayout;
		friend class Sampler;
		friend class QueryPool;
		friend class PipelineCache;
		friend class Pipeline;
		friend class RenderDoc;

		PhysicalDevice *parent;
		DeviceHndl hndl;
		std::map<uint32, vector<Queue>> queues;
		uint32 graphicsQueueFamily, transferQueueFamily;
		vector<const char*> enabledExtensions;

		PFN_vkDestroyDevice vkDestroyDevice;
		PFN_vkGetDeviceQueue vkGetDeviceQueue;
		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
		PFN_vkQueuePresentKHR vkQueuePresentKHR;
		PFN_vkQueueSubmit vkQueueSubmit;
		PFN_vkCreateSemaphore vkCreateSemaphore;
		PFN_vkDestroySemaphore vkDestroySemaphore;
		PFN_vkCreateCommandPool vkCreateCommandPool;
		PFN_vkDestroyCommandPool vkDestroyCommandPool;
		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
		PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
		PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
		PFN_vkEndCommandBuffer vkEndCommandBuffer;
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
		PFN_vkCmdClearColorImage vkCmdClearColorImage;
		PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
		PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
		PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
		PFN_vkCmdDraw vkCmdDraw;
		PFN_vkCmdBindPipeline vkCmdBindPipeline;
		PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
		PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
		PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
		PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
		PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
		PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
		PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
		PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
		PFN_vkCmdSetViewport vkCmdSetViewport;
		PFN_vkCmdSetScissor vkCmdSetScissor;
		PFN_vkCmdPushConstants vkCmdPushConstants;
		PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
		PFN_vkCmdNextSubpass vkCmdNextSubpass;
		PFN_vkCmdBeginQuery vkCmdBeginQuery;
		PFN_vkCmdEndQuery vkCmdEndQuery;
		PFN_vkCmdBlitImage vkCmdBlitImage;
		PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
		PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
		PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
		PFN_vkCreateRenderPass vkCreateRenderPass;
		PFN_vkDestroyRenderPass vkDestroyRenderPass;
		PFN_vkCreateShaderModule vkCreateShaderModule;
		PFN_vkDestroyShaderModule vkDestroyShaderModule;
		PFN_vkCreateImageView vkCreateImageView;
		PFN_vkDestroyImageView vkDestroyImageView;
		PFN_vkCreateFramebuffer vkCreateFramebuffer;
		PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
		PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
		PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
		PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
		PFN_vkDestroyPipeline vkDestroyPipeline;
		PFN_vkCreateFence vkCreateFence;
		PFN_vkDestroyFence vkDestroyFence;
		PFN_vkGetFenceStatus vkGetFenceStatus;
		PFN_vkResetFences vkResetFences;
		PFN_vkWaitForFences vkWaitForFences;
		PFN_vkCreateBuffer vkCreateBuffer;
		PFN_vkDestroyBuffer vkDestroyBuffer;
		PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
		PFN_vkAllocateMemory vkAllocateMemory;
		PFN_vkFreeMemory vkFreeMemory;
		PFN_vkBindBufferMemory vkBindBufferMemory;
		PFN_vkMapMemory vkMapMemory;
		PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
		PFN_vkUnmapMemory vkUnmapMemory;
		PFN_vkCreateImage vkCreateImage;
		PFN_vkDestroyImage vkDestroyImage;
		PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
		PFN_vkBindImageMemory vkBindImageMemory;
		PFN_vkCreateSampler vkCreateSampler;
		PFN_vkDestroySampler vkDestroySampler;
		PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
		PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
		PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
		PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
		PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
		PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
		PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
		PFN_vkCreateQueryPool vkCreateQueryPool;
		PFN_vkDestroyQueryPool vkDestroyQueryPool;
		PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
		PFN_vkQueueWaitIdle vkQueueWaitIdle;
		PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
		PFN_vkCreatePipelineCache vkCreatePipelineCache;
		PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
		PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
		PFN_vkMergePipelineCaches vkMergePipelineCaches;
		PFN_vkAcquireFullScreenExclusiveModeEXT vkAcquireFullScreenExclusiveModeEXT;
		PFN_vkReleaseFullScreenExclusiveModeEXT vkReleaseFullScreenExclusiveModeEXT;

		LogicalDevice(PhysicalDevice &parent, DeviceHndl hndl, const DeviceCreateInfo &createInfo);

#ifdef _DEBUG
		void SetDebugName(ObjectType type, const void *handle, const string &name);
		void BeginQueueLabel(QueueHndl queue, const DebugUtilsLabel &label);
		void EndQueueLabel(QueueHndl queue);
		void BeginCommandBufferLabel(CommandBufferHndl commandBuffer, const DebugUtilsLabel &label);
		void EndCommandBufferLabel(CommandBufferHndl commandBuffer);
#endif

		void LoadDeviceProcs(void);
		void Destory(void);
	};
}