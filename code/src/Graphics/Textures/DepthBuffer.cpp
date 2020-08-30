#include "Graphics/Textures/DepthBuffer.h"
#include "Graphics/Vulkan/CommandBuffer.h"

Pu::DepthBuffer::DepthBuffer(LogicalDevice & device, Format format, Extent2D size, uint32 layers)
	: Image(device, CreateImageInfo(format, size, layers))
{
	/* Set the image aspect and check if the format is valid. */
	SetAspect(format);
	view = new ImageView(*this, layers > 1 ? ImageViewType::Image2DArray : ImageViewType::Image2D, aspect);
}

Pu::DepthBuffer::DepthBuffer(DepthBuffer && value)
	: Image(std::move(value)), view(value.view)
{
	value.view = nullptr;
}

Pu::DepthBuffer & Pu::DepthBuffer::operator=(DepthBuffer && other)
{
	if (this != &other)
	{
		Destroy();
		Image::operator=(std::move(other));
		view = other.view;
		other.view = nullptr;
	}

	return *this;
}

void Pu::DepthBuffer::MakeWritable(CommandBuffer & cmdBuffer)
{
	/*
	The access mask needs to read write as we want to use it for writing and depth testing.
	The earliest stage where read/writes to this depth buffer can occur is the early fragment tests.
	*/
	cmdBuffer.MemoryBarrier(*this, PipelineStageFlags::TopOfPipe, PipelineStageFlags::EarlyFragmentTests, ImageLayout::DepthStencilAttachmentOptimal, AccessFlags::DepthStencilAttachmentReadWrite, GetFullRange(aspect));
}

/* layers, hide decleration in image, but this is not accessible anyways. */
#pragma warning(push)
#pragma warning(disable:4458)
Pu::ImageCreateInfo Pu::DepthBuffer::CreateImageInfo(Format depthFormat, Extent2D size, uint32 layers)
{
	return ImageCreateInfo
	{
		ImageType::Image2D,
		depthFormat,
		Extent3D(size, 1),
		1,
		layers,
		SampleCountFlags::Pixel1Bit,
		ImageUsageFlags::DepthStencilAttachment | ImageUsageFlags::TransientAttachment | ImageUsageFlags::InputAttachment
	};
}
#pragma warning(pop)

void Pu::DepthBuffer::SetAspect(Format depthFormat)
{
	switch (depthFormat)
	{
	case Format::D16_UNORM:
	case Format::X8_D24_UNORM_PACK32:
	case Format::D32_SFLOAT:
		aspect = ImageAspectFlags::Depth;
		break;
	case Format::S8_UINT:
	case Format::D16_UNORM_S8_UINT:
	case Format::D24_UNORM_S8_UINT:
	case Format::D32_SFLOAT_S8_UINT:
		aspect = ImageAspectFlags::Depth | ImageAspectFlags::Stencil;
		break;
	default:
		Log::Fatal("Non depth-stencil format passed to DepthBuffer!");
		return;
	}
}

void Pu::DepthBuffer::Destroy(void)
{
	if (view) delete view;
}