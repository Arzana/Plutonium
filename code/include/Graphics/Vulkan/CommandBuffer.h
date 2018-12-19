#pragma once
#include "VulkanGlobals.h"
#include "Shaders/GraphicsPipeline.h"
#include "Graphics/Color.h"
#include "Graphics/Vulkan/Framebuffer.h"

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
		CommandBufferHndl hndl;
		bool beginCalled;

		CommandBuffer(CommandPool &pool, CommandBufferHndl hndl);

		void Begin(void);
		void End(void);
		void Free(void);
	};
}