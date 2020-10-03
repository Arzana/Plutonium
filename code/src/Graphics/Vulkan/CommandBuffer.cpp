#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Vulkan/CommandPool.h"
#include "Graphics/Vulkan/DescriptorPool.h"

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

/* We really only want to check this in debug mode for safety. */
#ifdef _DEBUG
#define DbgCheckIfRecording(op) if (!CheckIfRecording(op)) return
#else
#define DbgCheckIfRecording(...)
#endif

static Pu::uint32 bindCalls = 0;
static Pu::uint32 drawCalls = 0;
static Pu::uint32 dispatchCalls = 0;
static Pu::uint32 transferCalls = 0;
static Pu::uint32 barrierCalls = 0;
static Pu::uint32 shaderCalls = 0;

Pu::CommandBuffer::CommandBuffer(void)
	: parent(nullptr), device(nullptr), hndl(nullptr), 
	state(State::Invalid), submitFence(nullptr), Usage(CommandBufferUsageFlags::None)
{}

Pu::CommandBuffer::CommandBuffer(CommandBuffer && value)
	: parent(value.parent), device(value.device), hndl(value.hndl), 
	state(value.state), submitFence(value.submitFence), Usage(value.Usage)
{
	value.hndl = nullptr;
	value.submitFence = nullptr;
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
		Usage = other.Usage;

		other.hndl = nullptr;
		other.submitFence = nullptr;
	}

	return *this;
}

bool Pu::CommandBuffer::CanBegin(bool wait) const
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

void Pu::CommandBuffer::CopyEntireBuffer(const Buffer & source, Buffer & destination)
{
	CopyBuffer(source, destination, BufferCopy{ 0, 0, source.GetSize() });
}

void Pu::CommandBuffer::CopyEntireBuffer(const Buffer & source, Image & destination)
{
	CopyBuffer(source, destination, BufferImageCopy{ destination.GetExtent() });
}

void Pu::CommandBuffer::CopyEntireImage(const Image & source, Buffer & destination)
{
	DbgCheckIfRecording("copt image to buffer");
	++transferCalls;

	const BufferImageCopy region(source.GetExtent());
	device->vkCmdCopyImageToBuffer(hndl, source.imageHndl, source.layout, destination.bufferHndl, 1, &region);
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & source, Buffer & destination, const vector<BufferCopy>& regions)
{
	DbgCheckIfRecording("copy buffer");
	++transferCalls;

	/* Append command to the command buffer and set the new element count for the buffer. */
	device->vkCmdCopyBuffer(hndl, source.bufferHndl, destination.bufferHndl, static_cast<uint32>(regions.size()), regions.data());
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & source, Image & destination, const vector<BufferImageCopy>& regions)
{
	DbgCheckIfRecording("copy buffer to image");
	++transferCalls;

	device->vkCmdCopyBufferToImage(hndl, source.bufferHndl, destination.imageHndl, destination.layout, static_cast<uint32>(regions.size()), regions.data());
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & source, Buffer & destination, const BufferCopy & region)
{
	DbgCheckIfRecording("copy buffer");
	++transferCalls;

	device->vkCmdCopyBuffer(hndl, source.bufferHndl, destination.bufferHndl, 1, &region);
}

void Pu::CommandBuffer::CopyBuffer(const Buffer & source, Image & destination, const BufferImageCopy & region)
{
	DbgCheckIfRecording("copy buffer to image");
	++transferCalls;

	device->vkCmdCopyBufferToImage(hndl, source.bufferHndl, destination.imageHndl, destination.layout, 1, &region);
}

void Pu::CommandBuffer::CopyImage(const Image & source, Buffer & destination, const vector<BufferImageCopy>& regions)
{
	DbgCheckIfRecording("copy image to buffer");
	++transferCalls;

	device->vkCmdCopyImageToBuffer(hndl, source.imageHndl, source.layout, destination.bufferHndl, static_cast<uint32>(regions.size()), regions.data());
}

void Pu::CommandBuffer::BlitImage(const Image & source, Image & destination, const ImageBlit & region, Filter filter)
{
	DbgCheckIfRecording("blit image");
	++transferCalls;

	device->vkCmdBlitImage(hndl, source.imageHndl, source.layout, destination.imageHndl, destination.layout, 1, &region, filter);
}

void Pu::CommandBuffer::BlitImage(const Image & source, ImageLayout srcLayout, Image & destination, ImageLayout dstLayout, const ImageBlit & region, Filter filter)
{
	DbgCheckIfRecording("blit image");
	++transferCalls;

	device->vkCmdBlitImage(hndl, source.imageHndl, srcLayout, destination.imageHndl, dstLayout, 1, &region, filter);
}

void Pu::CommandBuffer::MemoryBarrier(const Buffer & buffer, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, AccessFlags dstAccess, DependencyFlags dependencyFlags)
{
	DbgCheckIfRecording("setup buffer pipeline barrier");
	++barrierCalls;

	/* Create buffer memory barrier. */
	BufferMemoryBarrier barrier(buffer.bufferHndl);
	barrier.SrcAccessMask = buffer.srcAccess;
	barrier.DstAccessMask = dstAccess;

	/* Append the command. */
	device->vkCmdPipelineBarrier(hndl, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 1, &barrier, 0, nullptr);

	/* Set new buffer access. */
	buffer.srcAccess = dstAccess;
}

void Pu::CommandBuffer::MemoryBarrier(const Image & image, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, ImageLayout newLayout, AccessFlags dstAccess, ImageSubresourceRange range, DependencyFlags dependencyFlags, uint32 queueFamilyIndex)
{
	/* We can skip memory barriers that change no resources. */
	if (image.layout == newLayout && image.access == dstAccess) return;
	DbgCheckIfRecording("setup image pipeline barrier");
	++barrierCalls;

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

void Pu::CommandBuffer::MemoryBarrier(const vector<std::pair<const Image*, ImageSubresourceRange>>& images, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, ImageLayout newLayout, AccessFlags dstAccess, DependencyFlags dependencyFlags, uint32 queueFamiltyIndex)
{
	DbgCheckIfRecording("setup image pipeline barriers");
	++barrierCalls;

	/* Pre-allocate a buffer for the barriers. */
	vector<ImageMemoryBarrier> barriers;
	barriers.reserve(images.size());

	for (const auto &[img, range] : images)
	{
		/* Skip images that are already in the correct format. */
		if (img->layout != newLayout || img->access != dstAccess)
		{
			/* Create the memory barriers. */
			barriers.emplace_back(img->imageHndl, queueFamiltyIndex);
			ImageMemoryBarrier &cur = barriers.back();

			cur.SrcAccessMask = img->access;
			cur.DstAccessMask = dstAccess;
			cur.OldLayout = img->layout;
			cur.NewLayout = newLayout;
			cur.SubresourceRange = range;

			/* Set the new image access and layout mask. */
			img->access = dstAccess;
			img->layout = newLayout;
		}
	}

	/* Append the command. */
	device->vkCmdPipelineBarrier(hndl, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 0, nullptr, static_cast<uint32>(barriers.size()), barriers.data());
}

void Pu::CommandBuffer::ClearImage(Image & image, Color color)
{
	DbgCheckIfRecording("clear image");

	static const ImageSubresourceRange range(ImageAspectFlags::Color);
	device->vkCmdClearColorImage(hndl, image.imageHndl, image.layout, color.ToClearColor(), 1, &range);
}

void Pu::CommandBuffer::BeginRenderPass(const Renderpass & renderPass, const Framebuffer & framebuffer, SubpassContents contents)
{
	BeginRenderPass(renderPass, framebuffer, framebuffer.area, contents);
}

void Pu::CommandBuffer::BeginRenderPass(const Renderpass & renderPass, const Framebuffer & framebuffer, Rect2D renderArea, SubpassContents contents)
{
	DbgCheckIfRecording("begin render pass");
	BeginRenderPassInternal(renderPass.hndl, renderPass.clearValues, framebuffer, renderArea, contents);
}

void Pu::CommandBuffer::BindGraphicsPipeline(const GraphicsPipeline & pipeline)
{
	DbgCheckIfRecording("bind graphics pipeline");
	++bindCalls;

	device->vkCmdBindPipeline(hndl, PipelineBindPoint::Graphics, pipeline.Hndl);
}

void Pu::CommandBuffer::BindComputePipeline(const ComputePipeline & pipeline)
{
	DbgCheckIfRecording("bind compute pipeline");
	++bindCalls;

	device->vkCmdBindPipeline(hndl, PipelineBindPoint::Compute, pipeline.Hndl);
}

void Pu::CommandBuffer::BindVertexBuffer(uint32 binding, const Buffer & buffer, DeviceSize offset)
{
	DbgCheckIfRecording("bind vertex buffer");
	++bindCalls;

	device->vkCmdBindVertexBuffers(hndl, binding, 1, &buffer.bufferHndl, &offset);
}

void Pu::CommandBuffer::BindIndexBuffer(IndexType type, const Buffer & buffer, DeviceSize offset)
{
	DbgCheckIfRecording("bind index buffer");
	++bindCalls;

	device->vkCmdBindIndexBuffer(hndl, buffer.bufferHndl, offset, type);
}

void Pu::CommandBuffer::PushConstants(const Pipeline & pipeline, ShaderStageFlags stage, uint32 offset, size_t size, const void * constants)
{
	DbgCheckIfRecording("push constants");
	device->vkCmdPushConstants(hndl, pipeline.LayoutHndl, stage, offset, static_cast<uint32>(size), constants);
}

void Pu::CommandBuffer::BindGraphicsDescriptor(const Pipeline & pipeline, const DescriptorSet & descriptor)
{
	DbgCheckIfRecording("bind descriptor to graphics bind point");
	++bindCalls;

	device->vkCmdBindDescriptorSets(hndl, PipelineBindPoint::Graphics, pipeline.LayoutHndl, descriptor.set, 1, &descriptor.hndl, 0, nullptr);
}

void Pu::CommandBuffer::BindGraphicsDescriptors(const Pipeline & pipeline, uint32 subpassIdx, const DescriptorSetGroup & descriptors)
{
	DbgCheckIfRecording("bind descriptors to graphics bind point");

#ifdef _DEBUG
	const uint32 oldBindCalls = bindCalls;
#endif

	/* We need to bind all of the descriptors in the group that match the subpass index. */
	for (const auto[id, setHndl] : descriptors.hndls)
	{
		const uint32 subpass = static_cast<uint32>(id >> 32);
		if (subpass == subpassIdx)
		{
			const uint32 set = static_cast<uint32>(id & 0xFFFFFFFF);
			device->vkCmdBindDescriptorSets(hndl, PipelineBindPoint::Graphics, pipeline.LayoutHndl, set, 1, &setHndl, 0, nullptr);
			++bindCalls;
		}
	}

#ifdef _DEBUG
	/* This might occur if the user passes the wrong handle, it will probably crash later. */
	if (oldBindCalls == bindCalls) Log::Warning("Could not bind any Graphics DescriptorSet from DescriptorSetGroup!");
#endif
}

void Pu::CommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
	DbgCheckIfRecording("draw");
	++drawCalls;

	device->vkCmdDraw(hndl, vertexCount, instanceCount, firstVertex, firstInstance);
}

void Pu::CommandBuffer::Draw(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 firstInstance, int32 vertexOffset)
{
	DbgCheckIfRecording("draw");
	++drawCalls;

	device->vkCmdDrawIndexed(hndl, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Pu::CommandBuffer::Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ)
{
	DbgCheckIfRecording("dispatch");
	++dispatchCalls;

	device->vkCmdDispatch(hndl, groupCountX, groupCountY, groupCountZ);
}

void Pu::CommandBuffer::NextSubpass(SubpassContents contents)
{
	DbgCheckIfRecording("transition to next subpass");
	++shaderCalls;
	device->vkCmdNextSubpass(hndl, contents);
}

void Pu::CommandBuffer::EndRenderPass(void)
{
	DbgCheckIfRecording("end render pass");
	device->vkCmdEndRenderPass(hndl);
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

void Pu::CommandBuffer::WriteTimestamp(PipelineStageFlags stage, QueryPool & pool, uint32 queryIndex)
{
	DbgCheckIfRecording("write timestamp");
	device->vkCmdWriteTimestamp(hndl, stage, pool.hndl, queryIndex);
}

void Pu::CommandBuffer::BeginQuery(QueryPool & pool, uint32 query, QueryControlFlags flags)
{
	DbgCheckIfRecording("begin occlusion query");
	device->vkCmdBeginQuery(hndl, pool.hndl, query, flags);
}

void Pu::CommandBuffer::EndQuery(QueryPool & pool, uint32 query)
{
	DbgCheckIfRecording("end query");
	device->vkCmdEndQuery(hndl, pool.hndl, query);
}

void Pu::CommandBuffer::ResetQueries(QueryPool & pool)
{
	ResetQueries(pool, 0, pool.count);
}

void Pu::CommandBuffer::ResetQueries(QueryPool & pool, uint32 first, uint32 count)
{
	DbgCheckIfRecording("reset query pool");
	device->vkCmdResetQueryPool(hndl, pool.hndl, first, count);
}

void Pu::CommandBuffer::SetViewport(const Viewport& viewport)
{
	DbgCheckIfRecording("set viewport");
	device->vkCmdSetViewport(hndl, 0, 1, &viewport);
}

void Pu::CommandBuffer::SetScissor(Rect2D scissor)
{
	DbgCheckIfRecording("set scissor");
	device->vkCmdSetScissor(hndl, 0, 1, &scissor);
}

void Pu::CommandBuffer::SetViewportAndScissor(const Viewport & viewport)
{
	DbgCheckIfRecording("set viewport and scissor");

	const Rect2D scissor = viewport.GetScissor();
	device->vkCmdSetViewport(hndl, 0, 1, &viewport);
	device->vkCmdSetScissor(hndl, 0, 1, &scissor);
}

void Pu::CommandBuffer::SetLineWidth(float width)
{
	DbgCheckIfRecording("set dynamic line width");
	device->vkCmdSetLineWidth(hndl, width);
}

void Pu::CommandBuffer::SetLineStipple(uint32 factor, uint16 pattern)
{
	DbgCheckIfRecording("set dynamic line stipple");
	device->vkCmdSetLineStippleEXT(hndl, factor, pattern);
}

Pu::CommandBuffer::CommandBuffer(CommandPool & pool, CommandBufferHndl hndl)
	: parent(&pool), device(pool.parent), hndl(hndl), state(State::Initial), Usage(CommandBufferUsageFlags::None)
{
	submitFence = new Fence(*device);
}

void Pu::CommandBuffer::BeginRenderPassInternal(RenderPassHndl renderPass, const vector<ClearValue>& clearValues, const Framebuffer & framebuffer, Rect2D renderArea, SubpassContents contents)
{
	RenderPassBeginInfo info(renderPass, framebuffer.hndl, renderArea);
	info.ClearValueCount = static_cast<uint32>(clearValues.size());
	info.ClearValues = clearValues.data();

	++shaderCalls;
	device->vkCmdBeginRenderPass(hndl, &info, contents);
}

Pu::uint32 Pu::CommandBuffer::GetDrawCalls(void)
{
	return drawCalls;
}

Pu::uint32 Pu::CommandBuffer::GetDispatchCalls(void)
{
	return dispatchCalls;
}

Pu::uint32 Pu::CommandBuffer::GetBindCalls(void)
{
	return bindCalls;
}

Pu::uint32 Pu::CommandBuffer::GetTransferCalls(void)
{
	return transferCalls;
}

Pu::uint32 Pu::CommandBuffer::GetBarrierCalls(void)
{
	return barrierCalls;
}

Pu::uint32 Pu::CommandBuffer::GetShaderCalls(void)
{
	return shaderCalls;
}

void Pu::CommandBuffer::ResetCounters(void)
{
	drawCalls = 0;
	dispatchCalls = 0;
	bindCalls = 0;
	transferCalls = 0;
	barrierCalls = 0;
	shaderCalls = 0;
}

void Pu::CommandBuffer::Begin(void)
{
	/* Wait for the commandbuffer to be released by the device if it's been submitted before. */
	if (CanBegin(true))
	{
		const CommandBufferBeginInfo info{ Usage };
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

void Pu::CommandBuffer::Reset(void) const
{
	submitFence->Reset();
	state = State::Initial;
}

void Pu::CommandBuffer::Free(void)
{
	if (hndl) parent->FreeBuffer(hndl);
	if (submitFence) delete submitFence;
}

void Pu::CommandBuffer::Deallocate(void)
{
	Free();

	/* The method should be safe to call before the dtor, so set everything to null. */
	parent = nullptr;
	device = nullptr;
	submitFence = nullptr;
	hndl = nullptr;
	state = State::Invalid;
}

bool Pu::CommandBuffer::CheckIfRecording(const char * operation) const
{
	if (state == State::Recording) return true;

	Log::Error("Cannot %s on command buffer in %s state!", operation, ::to_string(state));
	return false;
}