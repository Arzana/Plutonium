#pragma once
#include "Graphics/Vulkan/CommandPool.h"

namespace Pu
{
	/* Defines a command buffer that can be easily created and used for a single queue submit. */
	class SingleUseCommandBuffer
		: public CommandBuffer
	{
	public:
		/* Initializes a new instance of a single use command buffer without initializing it. */
		SingleUseCommandBuffer(void);
		/* Initializes a new instance of a single use command buffer. */
		SingleUseCommandBuffer(_In_ LogicalDevice &device, _In_ uint32 queueFamilyIndex);
		SingleUseCommandBuffer(_In_ const SingleUseCommandBuffer&) = delete;
		/* Move constructor. */
		SingleUseCommandBuffer(_In_ SingleUseCommandBuffer &&value);
		/* Destroys the command buffer. */
		~SingleUseCommandBuffer(void)
		{
			Destroy();
		}

		_Check_return_ SingleUseCommandBuffer& operator =(_In_ const SingleUseCommandBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ SingleUseCommandBuffer& operator =(_In_ SingleUseCommandBuffer &&other);

		/* Initializes the command buffer. */
		void Initialize(_In_ LogicalDevice &device, _In_ uint32 queueFamilyIndex);

	private:
		CommandPool *pool;

		void Destroy(void);
	};
}