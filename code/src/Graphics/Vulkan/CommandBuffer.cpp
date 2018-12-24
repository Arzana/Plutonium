#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Vulkan/CommandPool.h"

Pu::CommandBuffer::CommandBuffer(CommandBuffer && value)
	: parent(value.parent), hndl(value.hndl), beginCalled(value.beginCalled)
{
	value.hndl = nullptr;
}

Pu::CommandBuffer & Pu::CommandBuffer::operator=(CommandBuffer && other)
{
	if (this != &other)
	{
		/* Make sure we end the command buffer and free it if needed. */
		if (beginCalled) End();
		Free();

		hndl = other.hndl;
		parent = std::move(other.parent);
		beginCalled = other.beginCalled;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::CommandBuffer::ClearImage(ImageHndl image, Color color, ImageLayout layout)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	if (beginCalled) parent.parent.vkCmdClearColorImage(hndl, image, layout, color.ToClearColor(), 1, &range);
	else Log::Warning("Cannot clear image on non-started CommandBuffer!");
}

void Pu::CommandBuffer::BeginRenderPass(const Renderpass & renderPass, const Framebuffer & framebuffer, Rect2D renderArea, SubpassContents contents)
{
	RenderPassBeginInfo info(renderPass.hndl, framebuffer.hndl, renderArea);
	info.ClearValueCount = static_cast<uint32>(renderPass.clearValues.size());
	info.ClearValues = renderPass.clearValues.data();
	
	if (beginCalled) parent.parent.vkCmdBeginRenderPass(hndl, &info, contents);
	else Log::Warning("Cannot begin render pass on non-started CommandBuffer!");
}

void Pu::CommandBuffer::BindGraphicsPipeline(const GraphicsPipeline & pipeline)
{
	if (beginCalled) parent.parent.vkCmdBindPipeline(hndl, PipelineBindPoint::Graphics, pipeline.hndl);
	else Log::Warning("Cannot bind graphics pipeline on non-started CommandBuffer!");
}

void Pu::CommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
	if (beginCalled) parent.parent.vkCmdDraw(hndl, vertexCount, instanceCount, firstVertex, firstInstance);
	else Log::Warning("Cannot draw on non-started CommandBuffer!");
}

void Pu::CommandBuffer::EndRenderPass(void)
{
	if (beginCalled) parent.parent.vkCmdEndRenderPass(hndl);
	else Log::Warning("Cannot end render pass on non-started CommandBuffer!");
}

Pu::CommandBuffer::CommandBuffer(CommandPool & pool, CommandBufferHndl hndl)
	: parent(pool), hndl(hndl), beginCalled(false)
{}

void Pu::CommandBuffer::Begin(void)
{
	static const CommandBufferBeginInfo info;

	if (!beginCalled)
	{
		beginCalled = true;
		VK_VALIDATE(parent.parent.vkBeginCommandBuffer(hndl, &info), PFN_vkBeginCommandBuffer);
	}
	else Log::Warning("Attempted to call begin on already begun CommandBuffer!");
}

void Pu::CommandBuffer::End(void)
{
	if (beginCalled)
	{
		beginCalled = false;
		VK_VALIDATE(parent.parent.vkEndCommandBuffer(hndl), PFN_vkEndCommandBuffer);
	}
	else Log::Warning("Attempted to call end on non-started CommandBuffer!");
}

void Pu::CommandBuffer::Free(void)
{
	if (hndl) parent.FreeBuffer(hndl);
}