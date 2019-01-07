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
		return "Invalid";
	}
}

Pu::CommandBuffer::CommandBuffer(CommandBuffer && value)
	: parent(value.parent), hndl(value.hndl), state(value.state), submitFence(value.submitFence)
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
		parent = std::move(other.parent);
		state = other.state;
		submitFence = other.submitFence;

		other.hndl = nullptr;
		other.submitFence = nullptr;
	}

	return *this;
}

void Pu::CommandBuffer::CopyEntireBuffer(const Buffer & srcBuffer, Buffer & dstBuffer)
{
	if (state == State::Recording)
	{
		const BufferCopy region(0, 0, srcBuffer.GetSize());

		/* Append command to the command buffer and set the new element count for the buffer. */
		parent.parent.vkCmdCopyBuffer(hndl, srcBuffer.bufferHndl, dstBuffer.bufferHndl, 1, &region);
		dstBuffer.elements = srcBuffer.elements;
	}
	else Log::Warning("Cannot copy buffer on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & srcBuffer, Buffer & dstBuffer, const vector<BufferCopy>& regions)
{
	if (state == State::Recording)
	{
		/* Append command to the command buffer and set the new element count for the buffer. */
		parent.parent.vkCmdCopyBuffer(hndl, srcBuffer.bufferHndl, dstBuffer.bufferHndl, static_cast<uint32>(regions.size()), regions.data());
		dstBuffer.elements = srcBuffer.elements;
	}
	else Log::Warning("Cannot copy buffer on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & source, Image & destination, const vector<BufferImageCopy>& regions)
{
	if (state == State::Recording)
	{
		parent.parent.vkCmdCopyBufferToImage(hndl, source.bufferHndl, destination.imageHndl, destination.layout, static_cast<uint32>(regions.size()), regions.data());
	}
	else Log::Warning("Cannot copy buffer to image on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::MemoryBarrier(const Buffer & buffer, PipelineStageFlag srcStageMask, PipelineStageFlag dstStageMask, DependencyFlag dependencyFlags, AccessFlag dstAccess)
{
	if (state == State::Recording)
	{
		/* Create buffer memory barrier. */
		BufferMemoryBarrier barrier(buffer.bufferHndl);
		barrier.SrcAccessMask = buffer.access;
		barrier.DstAccessMask = dstAccess;

		/* Append the command. */
		parent.parent.vkCmdPipelineBarrier(hndl, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 1, &barrier, 0, nullptr);

		/* Set new buffer access. */
		buffer.access = dstAccess;
	} 
	else Log::Warning("Cannot setup buffer pipeline barrier on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::MemoryBarrier(const Image & image, PipelineStageFlag srcStageMask, PipelineStageFlag dstStageMask, DependencyFlag dependencyFlags, ImageLayout newLayout, AccessFlag dstAccess, uint32 queueFamilyIndex, ImageSubresourceRange range)
{
	if (state == State::Recording)
	{
		/* Create the memory barrier. */
		ImageMemoryBarrier barrier(image.imageHndl, queueFamilyIndex);
		barrier.SrcAccessMask = image.access;
		barrier.DstAccessMask = dstAccess;
		barrier.OldLayout = image.layout;
		barrier.NewLayout = newLayout;
		barrier.SubresourceRange = range;

		/* Append the command. */
		parent.parent.vkCmdPipelineBarrier(hndl, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 0, nullptr, 1, &barrier);

		/* Set new image access and layout. */
		image.access = dstAccess;
		image.layout = newLayout;
	}
	else Log::Warning("Cannot setup image pipeline barrier on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::ClearImage(ImageHndl image, Color color, ImageLayout layout)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	if (state == State::Recording) parent.parent.vkCmdClearColorImage(hndl, image, layout, color.ToClearColor(), 1, &range);
	else Log::Warning("Cannot clear image on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::BeginRenderPass(const Renderpass & renderPass, const Framebuffer & framebuffer, Rect2D renderArea, SubpassContents contents)
{
	RenderPassBeginInfo info(renderPass.hndl, framebuffer.hndl, renderArea);
	info.ClearValueCount = static_cast<uint32>(renderPass.clearValues.size());
	info.ClearValues = renderPass.clearValues.data();

	if (state == State::Recording) parent.parent.vkCmdBeginRenderPass(hndl, &info, contents);
	else Log::Warning("Cannot begin render pass on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::BindGraphicsPipeline(const GraphicsPipeline & pipeline)
{
	if (state == State::Recording) parent.parent.vkCmdBindPipeline(hndl, PipelineBindPoint::Graphics, pipeline.hndl);
	else Log::Warning("Cannot bind graphics pipeline on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::BindVertexBuffer(uint32 binding, const Buffer & buffer, size_t offset)
{
	if (state == State::Recording) parent.parent.vkCmdBindVertexBuffers(hndl, binding, 1, &buffer.bufferHndl, static_cast<DeviceSize*>(&offset));
	else Log::Warning("Cannot bind vertex buffer on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::BindGraphicsDescriptor(const DescriptorSet & descriptor)
{
	if (state == State::Recording) parent.parent.vkCmdBindDescriptorSets(hndl, PipelineBindPoint::Graphics, descriptor.parent.parent.layoutHndl, 0, 1, &descriptor.hndl, 0, nullptr);
	else Log::Warning("Cannot bind descriptor to graphics bind point on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
	if (state == State::Recording) parent.parent.vkCmdDraw(hndl, vertexCount, instanceCount, firstVertex, firstInstance);
	else Log::Warning("Cannot draw on non-recording CommandBuffer!");
}

void Pu::CommandBuffer::EndRenderPass(void)
{
	if (state == State::Recording) parent.parent.vkCmdEndRenderPass(hndl);
	else Log::Warning("Cannot end render pass on non-recording CommandBuffer!");
}

Pu::CommandBuffer::CommandBuffer(CommandPool & pool, CommandBufferHndl hndl)
	: parent(pool), hndl(hndl), state(State::Initial)
{
	submitFence = new Fence(pool.parent);
}

void Pu::CommandBuffer::Begin(void)
{
	static const CommandBufferBeginInfo info;

	/* Wait for the commandbuffer to be released by the device if it's been submitted before. */
	if (state == State::Pending)
	{
		if (submitFence->Wait())
		{
			submitFence->Reset();
			state = State::Initial;
		}
	}

	if (state == State::Initial)
	{
		VK_VALIDATE(parent.parent.vkBeginCommandBuffer(hndl, &info), PFN_vkBeginCommandBuffer);
		state = State::Recording;
	}
	else Log::Error("Attempted to call begin on %s command buffer!", ::to_string(state));
}

void Pu::CommandBuffer::End(void)
{
	if (state == State::Recording)
	{
		VK_VALIDATE(parent.parent.vkEndCommandBuffer(hndl), PFN_vkEndCommandBuffer);
		state = State::Executable;
	}
	else Log::Error("Attempted to call end on %s command buffer!", ::to_string(state));
}

void Pu::CommandBuffer::Free(void)
{
	if (hndl) parent.FreeBuffer(hndl);
}