#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Vulkan/CommandPool.h"

const char* to_string(Pu::CommandBuffer::State state)
{
	switch (state)
	{
	case Pu::CommandBuffer::State::Initial:
		return "Initial";
	case Pu::CommandBuffer::State::Recording:
		return "Recording";
	case Pu::CommandBuffer::State::Executable:
		return "Executable";
	case Pu::CommandBuffer::State::Pending:
		return "Pending";
	case Pu::CommandBuffer::State::Invalid:
	default:
		return "Invalid";
	}
}

Pu::CommandBuffer::CommandBuffer(CommandBuffer && value)
	: parent(value.parent), device(value.device), hndl(value.hndl), state(value.state), submitFence(value.submitFence)
{
	value.hndl = nullptr;
	value.submitFence = nullptr;
}

Pu::CommandBuffer::~CommandBuffer(void)
{
	Free();
	if (submitFence) delete submitFence;
}

Pu::CommandBuffer & Pu::CommandBuffer::operator=(CommandBuffer && other)
{
	if (this != &other)
	{
		/* Make sure we end the command buffer and free it if needed. */
		Free();

		hndl = other.hndl;
		parent = other.parent;
		device = other.device;
		state = other.state;
		submitFence = other.submitFence;

		other.hndl = nullptr;
		other.submitFence = nullptr;
	}

	return *this;
}

bool Pu::CommandBuffer::CanBegin(bool wait)
{
	/* Early out for non-pending states. */
	switch (state)
	{
	case CommandBuffer::State::Initial:
		return true;
	case CommandBuffer::State::Recording:
	case CommandBuffer::State::Executable:
	case CommandBuffer::State::Invalid:
		return false;
	}

	/* Wait for the fence if requested. */
	if (wait)
	{
		if (submitFence->Wait())
		{
			Reset();
			return true;
		}

		/* Wait failed or timed out, invalidate command buffer. */
		state = State::Invalid;
		return false;
	}
	/* Quickly check if the fence is signaled if so reset the command buffer. */
	else if (submitFence->IsSignaled())
	{
		Reset();
		return true;
	}

	/* No wait happened and the fence wasn't signaled so just return false. */
	return false;
}

void Pu::CommandBuffer::CopyEntireBuffer(const Buffer & srcBuffer, Buffer & dstBuffer)
{
	if (CheckIfRecording("copy buffer"))
	{
		const BufferCopy region(0, 0, srcBuffer.GetSize());

		/* Append command to the command buffer and set the new element count for the buffer. */
		device->vkCmdCopyBuffer(hndl, srcBuffer.bufferHndl, dstBuffer.bufferHndl, 1, &region);
	}
}

void Pu::CommandBuffer::CopyEntireBuffer(const Buffer & source, Image & destination)
{
	if (CheckIfRecording("copy buffer to image"))
	{
		const BufferImageCopy region(destination.GetExtent());
		device->vkCmdCopyBufferToImage(hndl, source.bufferHndl, destination.imageHndl, destination.layout, 1, &region);
	}
}

void Pu::CommandBuffer::CopyEntireImage(const Image & source, Buffer & destination)
{
	if (CheckIfRecording("copy image to buffer"))
	{
		const BufferImageCopy region(source.GetExtent());
		device->vkCmdCopyImageToBuffer(hndl, source.imageHndl, source.layout, destination.bufferHndl, 1, &region);
	}
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & srcBuffer, Buffer & dstBuffer, const vector<BufferCopy>& regions)
{
	if (CheckIfRecording("copy buffer"))
	{
		/* Append command to the command buffer and set the new element count for the buffer. */
		device->vkCmdCopyBuffer(hndl, srcBuffer.bufferHndl, dstBuffer.bufferHndl, static_cast<uint32>(regions.size()), regions.data());
	}
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & source, Image & destination, const vector<BufferImageCopy>& regions)
{
	if (CheckIfRecording("copy buffer to image"))
	{
		device->vkCmdCopyBufferToImage(hndl, source.bufferHndl, destination.imageHndl, destination.layout, static_cast<uint32>(regions.size()), regions.data());
	}
}

void Pu::CommandBuffer::CopyImage(const Image & source, Buffer & destination, const vector<BufferImageCopy>& regions)
{
	if (CheckIfRecording("copy image to buffer"))
	{
		device->vkCmdCopyImageToBuffer(hndl, source.imageHndl, source.layout, destination.bufferHndl, static_cast<uint32>(regions.size()), regions.data());
	}
}

void Pu::CommandBuffer::MemoryBarrier(const Buffer & buffer, PipelineStageFlag srcStageMask, PipelineStageFlag dstStageMask, AccessFlag dstAccess, DependencyFlag dependencyFlags)
{
	if (CheckIfRecording("setup buffer pipeline barrier"))
	{
		/* Create buffer memory barrier. */
		BufferMemoryBarrier barrier(buffer.bufferHndl);
		barrier.SrcAccessMask = buffer.srcAccess;
		barrier.DstAccessMask = dstAccess;

		/* Append the command. */
		device->vkCmdPipelineBarrier(hndl, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 1, &barrier, 0, nullptr);

		/* Set new buffer access. */
		buffer.srcAccess = dstAccess;
	}
}

void Pu::CommandBuffer::MemoryBarrier(const Image & image, PipelineStageFlag srcStageMask, PipelineStageFlag dstStageMask, ImageLayout newLayout, AccessFlag dstAccess, ImageSubresourceRange range, DependencyFlag dependencyFlags, uint32 queueFamilyIndex)
{
	if (CheckIfRecording("setup image pipeline barrier"))
	{
		/* Create the memory barrier. */
		ImageMemoryBarrier barrier(image.imageHndl, queueFamilyIndex);
		barrier.SrcAccessMask = image.access;
		barrier.DstAccessMask = dstAccess;
		barrier.OldLayout = image.layout;
		barrier.NewLayout = newLayout;
		barrier.SubresourceRange = range;

		/* Append the command. */
		device->vkCmdPipelineBarrier(hndl, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 0, nullptr, 1, &barrier);

		/* Set new image access and layout. */
		image.access = dstAccess;
		image.layout = newLayout;
	}
}

void Pu::CommandBuffer::ClearImage(Image & image, Color color)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	if (CheckIfRecording("clear image")) device->vkCmdClearColorImage(hndl, image.imageHndl, image.layout, color.ToClearColor(), 1, &range);
}

void Pu::CommandBuffer::BeginRenderPass(const Renderpass & renderPass, const Framebuffer & framebuffer, SubpassContents contents)
{
	BeginRenderPass(renderPass, framebuffer, framebuffer.area, contents);
}

void Pu::CommandBuffer::BeginRenderPass(const Renderpass & renderPass, const Framebuffer & framebuffer, Rect2D renderArea, SubpassContents contents)
{
	if (CheckIfRecording("begin render pass"))
	{
		BeginRenderPassInternal(renderPass.hndl, renderPass.clearValues, framebuffer, renderArea, contents);
	}
}

void Pu::CommandBuffer::BindGraphicsPipeline(const GraphicsPipeline & pipeline)
{
	if (CheckIfRecording("bind graphics pipeline")) device->vkCmdBindPipeline(hndl, PipelineBindPoint::Graphics, pipeline.hndl);
}

void Pu::CommandBuffer::BindVertexBuffer(uint32 binding, const BufferView & view)
{
	if (CheckIfRecording("bind vertex buffer"))
	{
		device->vkCmdBindVertexBuffers(hndl, binding, 1, &view.buffer->bufferHndl, static_cast<const DeviceSize*>(&view.offset));
	}
}

void Pu::CommandBuffer::BindIndexBuffer(const BufferView & view, IndexType type)
{
	if (CheckIfRecording("bind index buffer"))
	{
		device->vkCmdBindIndexBuffer(hndl, view.buffer->bufferHndl, static_cast<DeviceSize>(view.offset), type);
	}
}

void Pu::CommandBuffer::PushConstants(const Renderpass & renderpass, ShaderStageFlag stage, size_t size, const void * constants)
{
	if (CheckIfRecording("push constants"))
	{
		device->vkCmdPushConstants(hndl, renderpass.layoutHndl, stage, 0, static_cast<uint32>(size), constants);
	}
}

void Pu::CommandBuffer::BindGraphicsDescriptor(const DescriptorSet & descriptor)
{
	if (CheckIfRecording("bind descriptor to graphics bind point"))
	{
		device->vkCmdBindDescriptorSets(hndl, PipelineBindPoint::Graphics, descriptor.parent->pipelineLayout, descriptor.set, 1, &descriptor.hndl, 0, nullptr);
	}
}

void Pu::CommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
	if (CheckIfRecording("draw")) device->vkCmdDraw(hndl, vertexCount, instanceCount, firstVertex, firstInstance);
}

void Pu::CommandBuffer::Draw(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 firstInstance, uint32 vertexOffset)
{
	if (CheckIfRecording("draw")) device->vkCmdDrawIndexed(hndl, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Pu::CommandBuffer::NextSubpass(SubpassContents contents)
{
	if (CheckIfRecording("transition to next subpass")) device->vkCmdNextSubpass(hndl, contents);
}

void Pu::CommandBuffer::EndRenderPass(void)
{
	if (CheckIfRecording("end render pass")) device->vkCmdEndRenderPass(hndl);
}

void Pu::CommandBuffer::AddLabel(const string & name, Color color)
{
#ifdef _DEBUG
	if (CheckIfRecording("add debug label"))
	{
		const Vector4 clr = color.ToVector4();

		DebugUtilsLabel label;
		label.LabelName = name.c_str();;
		memcpy(label.Color, &clr, sizeof(Vector4));

		device->BeginCommandBufferLabel(hndl, label);
	}
#else
	(void)name;
	(void)color;
#endif
}

void Pu::CommandBuffer::EndLabel(void)
{
#ifdef _DEBUG
	if (CheckIfRecording("end debug label")) device->EndCommandBufferLabel(hndl);
#endif
}

void Pu::CommandBuffer::WriteTimestamp(PipelineStageFlag stage, QueryPool & pool, uint32 queryIndex)
{
	if (CheckIfRecording("write timestamp")) device->vkCmdWriteTimestamp(hndl, stage, pool.hndl, queryIndex);
}

void Pu::CommandBuffer::BeginOcclusionQuery(QueryPool & pool, uint32 queryIndex, QueryControlFlag flags)
{
	if (CheckIfRecording("begin occlusion query")) device->vkCmdBeginQuery(hndl, pool.hndl, queryIndex, flags);
}

void Pu::CommandBuffer::EndQuery(QueryPool & pool, uint32 queryIndex)
{
	if (CheckIfRecording("end query")) device->vkCmdEndQuery(hndl, pool.hndl, queryIndex);
}

void Pu::CommandBuffer::SetViewport(const Viewport& viewport)
{
	if (CheckIfRecording("set viewport")) device->vkCmdSetViewport(hndl, 0, 1, &viewport);
}

void Pu::CommandBuffer::SetScissor(Rect2D scissor)
{
	if (CheckIfRecording("set scissor")) device->vkCmdSetScissor(hndl, 0, 1, &scissor);
}

void Pu::CommandBuffer::SetLineWidth(float width)
{
	if (CheckIfRecording("set dynamic line width")) device->vkCmdSetLineWidth(hndl, width);
}

Pu::CommandBuffer::CommandBuffer(CommandPool & pool, CommandBufferHndl hndl)
	: parent(&pool), device(pool.parent), hndl(hndl), state(State::Initial)
{
	submitFence = new Fence(*device);
}

void Pu::CommandBuffer::BeginRenderPassInternal(RenderPassHndl renderPass, const vector<ClearValue>& clearValues, const Framebuffer & framebuffer, Rect2D renderArea, SubpassContents contents)
{
	RenderPassBeginInfo info(renderPass, framebuffer.hndl, renderArea);
	info.ClearValueCount = static_cast<uint32>(clearValues.size());
	info.ClearValues = clearValues.data();

	device->vkCmdBeginRenderPass(hndl, &info, contents);
}

void Pu::CommandBuffer::Begin(void)
{
	static const CommandBufferBeginInfo info;

	/* Wait for the commandbuffer to be released by the device if it's been submitted before. */
	if (CanBegin(true))
	{
		VK_VALIDATE(device->vkBeginCommandBuffer(hndl, &info), PFN_vkBeginCommandBuffer);
		state = State::Recording;
	}
	else Log::Error("Attempted to call begin on %s command buffer!", ::to_string(state));
}

void Pu::CommandBuffer::End(void)
{
	if (state == State::Recording)
	{
		VK_VALIDATE(device->vkEndCommandBuffer(hndl), PFN_vkEndCommandBuffer);
		state = State::Executable;
	}
	else Log::Error("Attempted to call end on %s command buffer!", ::to_string(state));
}

void Pu::CommandBuffer::Reset(void)
{
	submitFence->Reset();
	state = State::Initial;
}

void Pu::CommandBuffer::Free(void)
{
	if (hndl) parent->FreeBuffer(hndl);
}

bool Pu::CommandBuffer::CheckIfRecording(const char * operation) const
{
	if (state == State::Recording) return true;

	Log::Error("Cannot %s on command buffer in %s state!", operation, ::to_string(state));
	return false;
}