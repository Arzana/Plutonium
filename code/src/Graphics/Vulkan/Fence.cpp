#include "Graphics/Vulkan/Fence.h"

Pu::Fence::Fence(LogicalDevice & device, bool signaled)
	: parent(&device)
{
	const FenceCreateInfo createInfo(signaled ? FenceCreateFlags::Signaled : FenceCreateFlags::None);
	VK_VALIDATE(parent->vkCreateFence(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateFence);
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
		
		parent = other.parent;
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
	const VkApiResult result = parent->vkGetFenceStatus(parent->hndl, hndl);
	if (result == VkApiResult::DeviceLost) Log::Error("Parent device of fence has been lost!");
	return result == VkApiResult::Success;
}

void Pu::Fence::Reset(void)
{
	VK_VALIDATE(parent->vkResetFences(parent->hndl, 1, &hndl), PFN_vkResetFences);
}

bool Pu::Fence::Wait(uint64 timeout) const
{
	return WaitInternal(*parent, 1, &hndl, true, timeout);
}

bool Pu::Fence::WaitAll(const LogicalDevice & device, const vector<const Fence*>& fences, uint64 timeout)
{
	const vector<FenceHndl> hndls = fences.select<FenceHndl>([](const Fence* const& fence) { return fence->hndl; });
	return WaitInternal(device, static_cast<uint32>(hndls.size()), hndls.data(), true, timeout);
}

bool Pu::Fence::WaitAny(const LogicalDevice & device, const vector<const Fence*>& fences, uint64 timeout)
{
	const vector<FenceHndl> hndls = fences.select<FenceHndl>([](const Fence* const& fence) { return fence->hndl; });
	return WaitInternal(device, static_cast<uint32>(hndls.size()), hndls.data(), false, timeout);
}

bool Pu::Fence::WaitInternal(const LogicalDevice & device, uint32 fenceCnt, const FenceHndl * fences, bool waitAll, uint64 timeout)
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
	VK_VALIDATE(result = device.vkWaitForFences(device.hndl, fenceCnt, fences, waitAll, timeout), PFN_vkWaitForFences);
	return result == VkApiResult::Success;
}

void Pu::Fence::Destroy(void)
{
	if (hndl) parent->vkDestroyFence(parent->hndl, hndl, nullptr);
}