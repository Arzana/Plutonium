#pragma once
#include "CommandBuffer.h"
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines an allocation pool for Vulkan commands. */
	class CommandPool
	{
	public:
		/* Initializes a new instance of a command pool from a specified queue family. */
		CommandPool(_In_ LogicalDevice &device, _In_ uint32 queueFamilyIndex, _In_ CommandPoolCreateFlag flags);
		CommandPool(_In_ const CommandPool&) = delete;
		/* Move constructor. */
		CommandPool(_In_ CommandPool &&value);
		/* Destroys the command pool. */
		~CommandPool(void)
		{
			Destroy();
		}

		_Check_return_ CommandPool& operator =(_In_ const CommandPool&) = delete;
		/* Move assignment. */
		_Check_return_ CommandPool& operator =(_In_ CommandPool &&other);

		/* Allocates a new command buffer from this pool. */
		_Check_return_ CommandBuffer Allocate(void) const;

	private:
		friend class CommandBuffer;

		LogicalDevice *parent;
		CommandPoolHndl hndl;

		void Destroy(void);
		void FreeBuffer(CommandBufferHndl commandBuffer) const;
	};
}