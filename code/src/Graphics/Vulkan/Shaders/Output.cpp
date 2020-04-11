#include "Graphics/Vulkan/Shaders/Output.h"

void Pu::Output::SetUsage(OutputUsage usage)
{
	if (usage == OutputUsage::Unknown)
	{
		Log::Error("Output field '%s' cannot be used as a %s attachment!", GetInfo().Name.c_str(), to_string(usage));
		return;
	}

	type = usage;
}

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
	reference.Layout = ImageLayout::ColorAttachmentOptimal;
	description.FinalLayout = ImageLayout::PresentSrcKhr;
}

void Pu::Output::SetDepthDescription(Format format)
{
	/* Set the format and default the clear value to the far plane and no stencil. */
	type = OutputUsage::DepthStencil;
	description.Format = format;
	SetClearValue({ 1.0f, 0 });
	SetLayouts(ImageLayout::DepthStencilAttachmentOptimal);
}

Pu::Output::Output(const FieldInfo & data, uint32 attachment, OutputUsage type)
	: Field(data), type(type), clear{0.0f, 0.0f, 0.0f, 0.0f}, reference(attachment, ImageLayout::General),
	description(Format::Undefined, ImageLayout::General, ImageLayout::General), clone(false)
{
#ifdef _DEBUG
	if (!data.Name.empty())
	{
		if (data.Storage != spv::StorageClass::Output) Log::Fatal("The output class cannot be used to store '%s'!", to_string(data.Storage));
	}
#endif
}