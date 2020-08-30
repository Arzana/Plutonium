#include "Graphics/Vulkan/CommandPool.h"
#include "Core/Diagnostics/Logging.h"

Pu::CommandPool::CommandPool(LogicalDevice & device, uint32 queueFamilyIndex, CommandPoolCreateFlags flags)
	: parent(&device)
{
	const CommandPoolCreateInfo createInfo(queueFamilyIndex, flags);
	VK_VALIDATE(parent->vkCreateCommandPool(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateCommandPool);
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
		parent = other.parent;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::CommandBuffer Pu::CommandPool::Allocate(void) const
{
	/* Initialize creation info. */
	const CommandBufferAllocateInfo allocInfo(hndl, 1);
	CommandBufferHndl commandBuffer;

	/* Allocate new buffer. */
	VK_VALIDATE(parent->vkAllocateCommandBuffers(parent->hndl, &allocInfo, &commandBuffer), PFN_vkAllocateCommandBuffers);
	return CommandBuffer(const_cast<CommandPool&>(*this), commandBuffer);
}

void Pu::CommandPool::Destroy(void)
{
	if (hndl) parent->vkDestroyCommandPool(parent->hndl, hndl, nullptr);
}

void Pu::CommandPool::FreeBuffer(CommandBufferHndl commandBuffer) const
{
	parent->vkFreeCommandBuffers(parent->hndl, hndl, 1, &commandBuffer);
}