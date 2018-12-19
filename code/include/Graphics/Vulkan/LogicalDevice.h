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

		/* Gets the specific queue created with the logical device. */
		_Check_return_ inline Queue& GetQueue(_In_ uint32 familyIndex, _In_ uint32 queueIndex)
		{
			return queues.at(familyIndex).at(queueIndex);
		}

		/* Gets the physical device the logical device was created on. */
		_Check_return_ inline const PhysicalDevice& GetPhysicalDevice(void) const
		{
			return parent;
		}

	private:
		friend class PhysicalDevice;
		friend class Semaphore;
		friend class Swapchain;
		friend class CommandPool;
		friend class Queue;
		friend class CommandBuffer;
		friend class GameWindow;
		friend class Subpass;
		friend class Renderpass;
		friend class ImageView;
		friend class Framebuffer;
		friend class GraphicsPipeline;

		PhysicalDevice &parent;
		DeviceHndl hndl;
		std::map<uint32, vector<Queue>> queues;

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

		LogicalDevice(PhysicalDevice &parent, DeviceHndl hndl, uint32 queueCreateInfoCount, const DeviceQueueCreateInfo *queueCreateInfos);

		void LoadDeviceProcs(void);
		void Destory(void);
	};
}