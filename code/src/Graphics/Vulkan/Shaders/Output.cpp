#include "Graphics/Vulkan/Shaders/Output.h"

void Pu::Output::SetColorBlending(BlendFactor srcFactor, BlendOp op, BlendFactor dstFactor)
{
	attachment.BlendEnable = true;
	attachment.SrcColorBlendFactor = srcFactor;
	attachment.DstColorBlendFactor = dstFactor;
	attachment.ColorBlendOp = op;
}

void Pu::Output::SetAlphaBlending(BlendFactor srcFactor, BlendOp op, BlendFactor dstFactor)
{
	attachment.BlendEnable = true;
	attachment.SrcAlphaBlendFactor = srcFactor;
	attachment.DstAlphaBlendFactor = dstFactor;
	attachment.AlphaBlendOp = op;
}

void Pu::Output::SetDescription(const Swapchain & swapchain)
{
	description.Format = swapchain.GetImageFormat();
	description.InitialLayout = ImageLayout::PresentSrcKhr;
	description.FinalLayout = ImageLayout::PresentSrcKhr;

	SetLayout(ImageLayout::ColorAttachmentOptimal);
}

void Pu::Output::SetDescription(const DepthBuffer & depthBuffer)
{
	description.Format = depthBuffer.GetFormat();
	description.InitialLayout = ImageLayout::Undefined;
	description.FinalLayout = ImageLayout::DepthStencilAttachmentOptimal;

	/* Default set the clear value to the far plane and no stencil if it's setup as a depth buffer. */
	SetLayout(ImageLayout::DepthStencilAttachmentOptimal);
	SetClearValue({ 1.0f, 0 });
}

Pu::Output::Output(const FieldInfo & data, uint32 attachment, OutputUsage type)
	: Field(data), type(type), resolve(false), clear{0.0f, 0.0f, 0.0f, 0.0f},
	reference(attachment, ImageLayout::General), description(Format::Undefined, ImageLayout::General, ImageLayout::General)
{
	if (data.Storage != spv::StorageClass::Output) Log::Fatal("The output class cannot be used to store '%s'!", to_string(data.Storage));
}