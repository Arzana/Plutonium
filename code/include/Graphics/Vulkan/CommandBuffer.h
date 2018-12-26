#pragma once
#include "Shaders/GraphicsPipeline.h"
#include "Graphics/Color.h"
#include "Framebuffer.h"
#include "Fence.h"

namespace Pu
{
	class CommandPool;

	/* Defines a buffer for commands. */
	class CommandBuffer
	{
	public:
		/* Defines all possible states of a command buffer. */
		enum class State
		{
			/* Indicates the default state of the command buffer. */
			Initial,
			/* Indicates the command buffer is able to record commands. */
			Recording,
			/* Indicates that the command buffer can be submitted, reset or recorded into another command buffer. */
			Executable,
			/* Indicates the command buffer has been submitted to a queue and is pending result. */
			Pending,
			/* Indicates an invalid state of the command buffer. */
			Invalid
		};

		CommandBuffer(_In_ const CommandBuffer&) = delete;
		/* Move constructor. */
		CommandBuffer(_In_ CommandBuffer &&value);
		/* Frees the command buffer. */
		~CommandBuffer(void);

		_Check_return_ CommandBuffer& operator =(_In_ const CommandBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ CommandBuffer& operator =(_In_ CommandBuffer &&other);

		/* Appends an image clear command to the command buffer. */
		void ClearImage(_In_ ImageHndl image, _In_ Color color, _In_opt_ ImageLayout layout = ImageLayout::TransferDstOptimal);
		/* Appends a render pass begin command to the command buffer. */
		void BeginRenderPass(_In_ const Renderpass &renderPass, _In_ const Framebuffer &framebuffer, _In_ Rect2D renderArea, _In_ SubpassContents contents);
		/* Appends a graphics pipeline bind command to the command buffer. */
		void BindGraphicsPipeline(_In_ const GraphicsPipeline &pipeline);
		/* Appends a draw command to the command buffer. */
		void Draw(_In_ uint32 vertexCount, _In_ uint32 instanceCount, _In_ uint32 firstVertex, _In_ uint32 firstInstance);
		/* Appends a render pass end command to the command buffer. */
		void EndRenderPass(void);

	private:
		friend class CommandPool;
		friend class Queue;
		friend class GameWindow;

		CommandPool &parent;
		Fence *submitFence;
		CommandBufferHndl hndl;
		State state;

		CommandBuffer(CommandPool &pool, CommandBufferHndl hndl);

		void Begin(void);
		void End(void);
		void Free(void);
	};
}