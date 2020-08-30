#include "Graphics/Resources/SingleUseCommandBuffer.h"

Pu::SingleUseCommandBuffer::SingleUseCommandBuffer(void)
	: pool(nullptr)
{}

Pu::SingleUseCommandBuffer::SingleUseCommandBuffer(LogicalDevice & device, uint32 queueFamilyIndex)
	: pool(nullptr)
{
	Initialize(device, queueFamilyIndex);
}

Pu::SingleUseCommandBuffer::SingleUseCommandBuffer(SingleUseCommandBuffer && value)
	: CommandBuffer(std::move(value)), pool(value.pool)
{
	value.pool = nullptr;
}

Pu::SingleUseCommandBuffer & Pu::SingleUseCommandBuffer::operator=(SingleUseCommandBuffer && other)
{
	if (this != &other)
	{
		Destroy();
		CommandBuffer::operator=(std::move(other));
		pool = other.pool;
		other.pool = nullptr;
	}

	return *this;
}

#pragma warning(push)
#pragma warning(disable:4458)
void Pu::SingleUseCommandBuffer::Initialize(LogicalDevice & device, uint32 queueFamilyIndex)
{
	/* Quick error check. */
	if (pool)
	{
		Log::Warning("Attempting to initialize already initialized single use command buffer!");
		return;
	}

	/* Indicate to the pool that the command buffer will be short lived and will only be submitted once. */
	pool = new CommandPool(device, queueFamilyIndex, CommandPoolCreateFlags::Transient);

	CommandBuffer::operator=(std::move(pool->Allocate()));
	Usage = CommandBufferUsageFlags::OneTimeSubmit;
}
#pragma warning(pop)

void Pu::SingleUseCommandBuffer::Destroy(void)
{
	Deallocate();
	if (pool) delete pool;
}