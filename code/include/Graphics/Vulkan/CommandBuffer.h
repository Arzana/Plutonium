#pragma once
#include "Graphics/Resources/BufferView.h"
#include "Shaders/GraphicsPipeline.h"
#include "Graphics/Color.h"
#include "Framebuffer.h"
#include "QueryPool.h"
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

		/* Initializes an invalid instance of a command buffer. */
		CommandBuffer(void);
		CommandBuffer(_In_ const CommandBuffer&) = delete;
		/* Move constructor. */
		CommandBuffer(_In_ CommandBuffer &&value);
		/* Deallocates the command buffer. */
		~CommandBuffer(void)
		{
			Free();
		}

		_Check_return_ CommandBuffer& operator =(_In_ const CommandBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ CommandBuffer& operator =(_In_ CommandBuffer &&other);

		/* Checks whether two command buffers are equal. */
		_Check_return_ inline bool operator ==(_In_ const CommandBuffer &other) const
		{
			return other.hndl == hndl;
		}

		/* Checks whether two command buffers differ. */
		_Check_return_ inline bool operator !=(_In_ const CommandBuffer &other) const
		{
			return other.hndl != hndl;
		}

		/* Deallocates the command buffer from its parent pool. */
		void Deallocate(void);
		/* Gets whether the command buffer is in an initial state, optionally waits for the command buffer to be signaled. */
		_Check_return_ bool CanBegin(_In_opt_ bool wait = false) const;
		/* Appends a copy command for the entire source buffer into the destination buffer to the command buffer. */
		void CopyEntireBuffer(_In_ const Buffer &srcBuffer, _In_ Buffer &dstBuffer);
		/* Appends a copy command for the entire source buffer into the destination image to the command buffer. */
		void CopyEntireBuffer(_In_ const Buffer &source, _In_ Image &destination);
		/* Appends a copy command for the entire source image into the destination buffer to the command buffer. */
		void CopyEntireImage(_In_ const Image &source, _In_ Buffer &destination);
		/* Appends a copy command from the source buffer to the destination buffer to the command buffer. */
		void CopyBuffer(_In_ const Buffer &srcBuffer, _In_ Buffer &dstBuffer, _In_ const vector<BufferCopy> &regions);
		/* Appends a copy command from the source buffer to the destination image to the command buffer. */
		void CopyBuffer(_In_ const Buffer &source, _In_ Image &destination, _In_ const vector<BufferImageCopy> &regions);
		/* Appends a copy command from the source image to the destination buffer to the command buffer. */
		void CopyImage(_In_ const Image &source, _In_ Buffer &destination, _In_ const vector<BufferImageCopy> &regions);
		/* Appends a pipeline buffer memory barrier command to the command buffer. */
		void MemoryBarrier(_In_ const Buffer &buffer, _In_ PipelineStageFlag srcStageMask, _In_ PipelineStageFlag dstStageMask, _In_ AccessFlag dstAccess, _In_ DependencyFlag dependencyFlags = DependencyFlag::None);
		/* Appends a pipeline image memory barrier command to the command buffer. */
		void MemoryBarrier(_In_ const Image &image, _In_ PipelineStageFlag srcStageMask, _In_ PipelineStageFlag dstStageMask, _In_ ImageLayout newLayout, _In_ AccessFlag dstAccess, _In_ ImageSubresourceRange range, _In_ DependencyFlag dependencyFlags = DependencyFlag::None, _In_ uint32 queueFamilyIndex = QueueFamilyIgnored);
		/* Appends an image clear command to the command buffer. */
		void ClearImage(_In_ Image &image, _In_ Color color);
		/* Appends a render pass begin command for the entire framebuffer to the command buffer. */
		void BeginRenderPass(_In_ const Renderpass &renderPass, _In_ const Framebuffer &framebuffer, _In_ SubpassContents contents);
		/* Appends a render pass begin command to the command buffer. */
		void BeginRenderPass(_In_ const Renderpass &renderPass, _In_ const Framebuffer &framebuffer, _In_ Rect2D renderArea, _In_ SubpassContents contents);
		/* Appends a graphics pipeline bind command to the command buffer. */
		void BindGraphicsPipeline(_In_ const GraphicsPipeline &pipeline);
		/* Appends a vertex buffer bind command to the command buffer. */
		void BindVertexBuffer(_In_ uint32 binding, _In_ const BufferView &view);
		/* Appends a index buffer bind command to the command buffer. */
		void BindIndexBuffer(_In_ const BufferView &view, _In_ IndexType type);
		/* Appends a update for the push constants to the command buffer. */
		void PushConstants(_In_ const Renderpass &renderpass, _In_ ShaderStageFlag stage, _In_ size_t size, _In_ const void *constants);
		/* Appends a graphics descriptor bind command to the command buffer. */
		void BindGraphicsDescriptor(_In_ const DescriptorSet &descriptor);
		/* Appends a draw command to the command buffer. */
		void Draw(_In_ uint32 vertexCount, _In_ uint32 instanceCount, _In_ uint32 firstVertex, _In_ uint32 firstInstance);
		/* Appends an indexed draw command to the command buffer. */
		void Draw(_In_ uint32 indexCount, _In_ uint32 instanceCount, _In_ uint32 firstIndex, _In_ uint32 firstInstance, _In_ uint32 vertexOffset);
		/* Transitions to the next subpass of a render pass. */
		void NextSubpass(_In_ SubpassContents contents);
		/* Appends a render pass end command to the command buffer. */
		void EndRenderPass(void);
		/* Appends a debug label to the command buffer (only active on debug). */
		void AddLabel(_In_ const string &name, _In_ Color color);
		/* Ends the last added label in the command buffer (only active on debug). */
		void EndLabel(void);
		/* Writes a timestamp at a specific point in the pipeline to the specific query. */
		void WriteTimestamp(_In_ PipelineStageFlag stage, _In_ QueryPool &pool, _In_ uint32 queryIndex);
		/* Starts an occlusion query. */
		void BeginOcclusionQuery(_In_ QueryPool &pool, _In_ uint32 queryIndex, _In_opt_ QueryControlFlag flags = QueryControlFlag::None);
		/* Ends a query. */
		void EndQuery(_In_ QueryPool &pool, _In_ uint32 queryIndex);
		/* Sets the viewport information for a graphics pipeline with dynamic state viewports. */
		void SetViewport(_In_ const Viewport &viewport);
		/* Sets the scissor rectangle for a graphics pipeline with dynamic state scissors. */
		void SetScissor(_In_ Rect2D scissor);
		/* Sets the dynamic line width state. */
		void SetLineWidth(_In_ float width);

	private:
		friend class CommandPool;
		friend class Queue;
		friend class GameWindow;
		friend class AssetLoader;
		friend class AssetSaver;

		CommandPool *parent;
		LogicalDevice *device;
		Fence *submitFence;
		CommandBufferHndl hndl;
		mutable State state;

		CommandBuffer(CommandPool &pool, CommandBufferHndl hndl);

		void BeginRenderPassInternal(RenderPassHndl renderPass, const vector<ClearValue> &clearValues, const Framebuffer &framebuffer, Rect2D renderArea, SubpassContents contents);
		void Begin(void);
		void End(void);
		void Reset(void) const;
		void Free(void);
		bool CheckIfRecording(_In_ const char *operation) const;
	};
}