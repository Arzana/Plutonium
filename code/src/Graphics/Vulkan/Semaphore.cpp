#include "Graphics/Vulkan/Semaphore.h"
#include "Core/Diagnostics/Logging.h"

Pu::Semaphore::Semaphore(LogicalDevice & device)
	: parent(device)
{
	const SemaphoreCreateInfo info;
	VK_VALIDATE(parent.vkCreateSemaphore(parent.hndl, &info, nullptr, &hndl), PFN_vkCreateSemaphore);
}

Pu::Semaphore::Semaphore(Semaphore && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::Semaphore & Pu::Semaphore::operator=(Semaphore && other)
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

void Pu::Semaphore::Destroy(void)
{
	if (hndl) parent.vkDestroySemaphore(parent.hndl, hndl, nullptr);
}