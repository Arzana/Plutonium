#include "Graphics/Textures/DefaultTexture.h"
#include "Graphics/Vulkan/CommandBuffer.h"

Pu::DefaultTexture::DefaultTexture(LogicalDevice & device, Color color)
	: Texture(CreateAndSetSampler(device), CreateAndSetImage(device)), buffer(nullptr), color(color)
{}

Pu::DefaultTexture::DefaultTexture(DefaultTexture && value)
	: Texture(std::move(value)), buffer(value.buffer), color(value.color),
	resSampler(value.resSampler), resImage(value.resImage)
{
	value.buffer = nullptr;
	value.resSampler = nullptr;
	value.resImage = nullptr;
}

Pu::DefaultTexture & Pu::DefaultTexture::operator=(DefaultTexture && other)
{
	if (this != &other)
	{
		Destroy();
		Texture::operator=(std::move(other));
		buffer = other.buffer;
		resSampler = other.resSampler;
		resImage = other.resImage;
		color = other.color;

		other.buffer = nullptr;
		other.resSampler = nullptr;
		other.resImage = nullptr;
	}

	return *this;
}

void Pu::DefaultTexture::Initialize(CommandBuffer & cmdBuffer)
{
	/* Only create a new buffer if needed. */
	if (!buffer) buffer = new StagingBuffer(resImage->GetDevice(), 4);

	/* Copy our single color into the staging buffer, and copy it to the texture. */
	buffer->Load(color.ToArray());
	cmdBuffer.MemoryBarrier(*resImage, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, GetFullRange());
	cmdBuffer.CopyEntireBuffer(*buffer, *resImage);
}

void Pu::DefaultTexture::Finalize(CommandBuffer & cmdBuffer)
{
	/* Check for invalid callers. */
	if (!buffer)
	{
		Log::Warning("Attempting to finalize default texture that is already finalized or is not yet initialized!");
		return;
	}

	/* Delete and reset the staging buffer. */
	delete buffer;
	buffer = nullptr;

	/* Make the image ready fo shader reads, if needed. */
	if (resImage->GetLayout() != ImageLayout::ShaderReadOnlyOptimal)
	{
		cmdBuffer.MemoryBarrier(*resImage, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, GetFullRange());
		MarkImage();
	}
}

void Pu::DefaultTexture::Create(CommandBuffer & cmdBuffer)
{
	/* Copy the data into the staging buffer. */
	if (!buffer) buffer = new StagingBuffer(resImage->GetDevice(), 4);
	buffer->Load(color.ToArray());

	/* Copy the data from the CPU buffer into the GPU buffer (the staging buffer will be deleted later). */
	cmdBuffer.MemoryBarrier(*resImage, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, GetFullRange());
	cmdBuffer.CopyEntireBuffer(*buffer, *resImage);
	cmdBuffer.MemoryBarrier(*resImage, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, GetFullRange());
	MarkImage();
}

Pu::Sampler & Pu::DefaultTexture::CreateAndSetSampler(LogicalDevice & device)
{
	/* The sampler info doesn't really matter so just set it to nearest and repeat to make sure we always have the color. */
	const SamplerCreateInfo info(Filter::Nearest, SamplerMipmapMode::Nearest, SamplerAddressMode::Repeat);
	return *(resSampler = new Pu::Sampler(device, info));
}

Pu::Image & Pu::DefaultTexture::CreateAndSetImage(LogicalDevice & device)
{
	/* RGB format is not available so we just go RGBA, multisampling doesn't make sense and the image is 1x1. */
	const ImageCreateInfo info(ImageType::Image2D, Format::R8G8B8A8_UNORM, Extent3D(1, 1, 1), 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled);
	return *(resImage = new Pu::Image(device, info));
}

void Pu::DefaultTexture::MarkImage(void)
{
	/* We should mark the underlying image as a loaded resource. */
	wstring name = L"Default Image (";
	name += static_cast<string>(color).toWide() += ')';
	resImage->MarkAsLoaded(false, std::move(name));
}

void Pu::DefaultTexture::Destroy(void)
{
	/* 
	The sampler and image might be moved away and the buffer might still be active from staging.
	If this happens we need to wait for the staging to be done, to be sure we don't delete an in flight resource.
	*/
	if (buffer)
	{
		resImage->GetDevice().WaitIdle();
		delete buffer;
	}

	if (resSampler) delete resSampler;
	if (resImage) delete resImage;
}