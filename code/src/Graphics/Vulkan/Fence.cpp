#include "Graphics/Vulkan/Fence.h"

Pu::Fence::Fence(LogicalDevice & device, bool signaled)
	: parent(device)
{
	const FenceCreateInfo createInfo(signaled ? FenceCreateFlag::Signaled : FenceCreateFlag::None);
	VK_VALIDATE(parent.vkCreateFence(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateFence);
}

Pu::Fence::Fence(Fence && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::Fence & Pu::Fence::operator=(Fence && other)
{
	if (this != &other)
	{
		Destroy();
		
		parent = std::move(other.parent);
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

bool Pu::Fence::IsSignaled(void) const
{
	/*
	Possible results:
	- VK_SUCCESS:			The fence has been signaled.
	- VK_NOT_READY:			The fence is unsignaled.
	- VK_ERROR_DEVICE_LOST:	The device parent has been lost.
	*/
	const VkApiResult result = parent.vkGetFenceStatus(parent.hndl, hndl);
	if (result == VkApiResult::DeviceLost) Log::Error("Parent device of fence has been lost!");
	return result == VkApiResult::Success;
}

void Pu::Fence::Reset(void)
{
	VK_VALIDATE(parent.vkResetFences(parent.hndl, 1, &hndl), PFN_vkResetFences);
}

bool Pu::Fence::Wait(uint64 timeout) const
{
	/*
	Possible results:
	- VK_SUCCESS:						The fence was signaled.
	- VK_TIMEOUT:						The wait duration has timed out.
	- VK_ERROR_OUT_OF_HOST_MEMORY:		Unable to allocate memory from host heap.
	- VK_ERROR_OUT_OF_DEVICE_MEMORY:	Unable to allocate memory from device heap.
	- VK_ERROR_DEVICE_LOST:				The device parent has been lost.
	*/
	VkApiResult result;
	VK_VALIDATE(result = parent.vkWaitForFences(parent.hndl, 1, &hndl, true, timeout), PFN_vkWaitForFences);
	return result == VkApiResult::Success;
}

void Pu::Fence::Destroy(void)
{
	if (hndl) parent.vkDestroyFence(parent.hndl, hndl, nullptr);
}