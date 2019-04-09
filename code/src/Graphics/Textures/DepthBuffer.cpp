#include "Graphics/Textures/DepthBuffer.h"
#include "Graphics/Vulkan/CommandBuffer.h"

Pu::DepthBuffer::DepthBuffer(LogicalDevice & device, Format format, Extent2D size)
	: Image(device, CreateImageInfo(format, size))
{
	/* Set the image aspect and check if the format is valid. */
	SetAspect(format);
	view = new ImageView(*this, aspect);
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
	These read operations happen at the early fragment test and the writes at the late fragment test.
	*/
	const ImageSubresourceRange range(aspect);
	cmdBuffer.MemoryBarrier(*this, PipelineStageFlag::TopOfPipe, PipelineStageFlag::EarlyFragmentTests, ImageLayout::DepthStencilAttachmentOptimal, AccessFlag::DepthStencilAttachmentReadWrite, range);
}

Pu::ImageCreateInfo Pu::DepthBuffer::CreateImageInfo(Format depthFormat, Extent2D size)
{
	return ImageCreateInfo
	{
		ImageType::Image2D,
		depthFormat,
		Extent3D(size, 1),
		1,
		1,
		SampleCountFlag::Pixel1Bit,
		ImageUsageFlag::DepthStencilAttachment
	};
}

void Pu::DepthBuffer::SetAspect(Format depthFormat)
{
	switch (depthFormat)
	{
	case Format::D16_UNORM:
	case Format::X8_D24_UNORM_PACK32:
	case Format::D32_SFLOAT:
		aspect = ImageAspectFlag::Depth;
		break;
	case Format::S8_UINT:
	case Format::D16_UNORM_S8_UINT:
	case Format::D24_UNORM_S8_UINT:
	case Format::D32_SFLOAT_S8_UINT:
		aspect = ImageAspectFlag::Depth | ImageAspectFlag::Stencil;
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