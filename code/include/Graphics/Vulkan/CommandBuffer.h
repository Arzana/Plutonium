#pragma once
#include "VulkanGlobals.h"
#include "Graphics/Color.h"

namespace Pu
{
	class CommandPool;

	/* Defines a buffer for commands. */
	class CommandBuffer
	{
	public:
		CommandBuffer(_In_ const CommandBuffer&) = delete;
		/* Move constructor. */
		CommandBuffer(_In_ CommandBuffer &&value);
		/* Frees the command buffer. */
		~CommandBuffer(void)
		{
			Free();
		}

		_Check_return_ CommandBuffer& operator =(_In_ const CommandBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ CommandBuffer& operator =(_In_ CommandBuffer &&other);

		/* Appends an image clear command to the command buffer. */
		void ClearImage(_In_ ImageHndl image, _In_ Color color, _In_opt_ ImageLayout layout = ImageLayout::TransferDstOptimal);

	private:
		friend class CommandPool;
		friend class Queue;
		friend class GameWindow;

		CommandPool &parent;
		CommandBufferHndl hndl;

		CommandBuffer(CommandPool &pool, CommandBufferHndl hndl);

		void Free(void);
	};
}