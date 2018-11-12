#include "Graphics/Vulkan/CommandPool.h"
#include "Core/Diagnostics/Logging.h"

Pu::CommandPool::CommandPool(LogicalDevice & device, uint32 queueFamilyIndex)
	: parent(device)
{
	const CommandPoolCreateInfo createInfo(queueFamilyIndex);
	const VkApiResult result = parent.vkCreateCommandPool(parent.hndl, &createInfo, nullptr, &hndl);
	if (result != VkApiResult::Success) Log::Fatal("Unable to create command pool!");
}

Pu::CommandPool::CommandPool(CommandPool && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::CommandPool & Pu::CommandPool::operator=(CommandPool && other)
{
	if (this != &other)
	{
		Destroy();
		hndl = other.hndl;
		parent = std::move(other.parent);

		other.hndl = nullptr;
	}

	return *this;
}

Pu::CommandBuffer Pu::CommandPool::AllocateCommandBuffer(void) const
{
	/* Initialize creation info. */
	const CommandBufferAllocateInfo allocInfo(hndl, 1);
	CommandBufferHndl commandBuffer;

	/* Allocate new buffer. */
	const VkApiResult result = parent.vkAllocateCommandBuffers(parent.hndl, &allocInfo, &commandBuffer);
	if (result != VkApiResult::Success) Log::Fatal("Unable to create command buffer!");

	/* Return new buffer object. */
	return CommandBuffer(const_cast<CommandPool&>(*this), commandBuffer);
}

void Pu::CommandPool::Destroy(void)
{
	if (hndl) parent.vkDestroyCommandPool(parent.hndl, hndl, nullptr);
}

void Pu::CommandPool::FreeBuffer(CommandBufferHndl commandBuffer) const
{
	parent.vkFreeCommandBuffers(parent.hndl, hndl, 1, &commandBuffer);
}