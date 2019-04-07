#include "Content/AssetSaver.h"
#include "Graphics/Vulkan/CommandPool.h"
#include "Streams/FileWriter.h"
#include "Core/Diagnostics/DbgUtils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#define STBI_MSC_SECURE_CRT

#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#else
#define STBIW_ASSERT(x)	if(!(x)) { Pu::Log::Fatal("STB image writer raised an error!"); }
#endif

#include <stb/stb/stb_image_write.h>

Pu::AssetSaver::AssetSaver(TaskScheduler & scheduler, LogicalDevice & device)
	: scheduler(scheduler), device(device), transferQueue(device.GetTransferQueue(0))
{
	cmdPool = new CommandPool(device, transferQueue.GetFamilyIndex());

	for (size_t i = 0; i < InitialLoadCommandBufferCount; i++)
	{
		AllocateCmdBuffer();
	}
}

Pu::AssetSaver::~AssetSaver(void)
{
	buffers.clear();
	delete cmdPool;
}

void Pu::AssetSaver::SaveImage(const Image & image, const wstring & path, ImageSaveFormats format)
{
	class SaveTask
		: public Task
	{
	public:
		SaveTask(AssetSaver &parent, const Image &image, const wstring &path, ImageSaveFormats format)
			: parent(parent), image(image), path(path), format(format), cmdBuffer(parent.GetCmdBuffer())
		{}

		virtual Result Execute(void) override
		{
			/* Creat the buffer needed as a result for the image data. */
			const Extent3D extent = image.GetExtent();
			const size_t imageSizeBytes = extent.Width * extent.Height * extent.Depth * image.GetChannels();
			const ImageSubresourceRange range(ImageAspectFlag::Color);

			destination = new Buffer(parent.device, imageSizeBytes, BufferUsageFlag::TransferDst, true);

			/* 
			Begin the command buffer and ensure a usable layouts.
			We can only start this the first memory barrier at transfer because the image source access is probably transfer destination.
			The buffer on the other hand has no access flag set so we can do it at the top of pipe already.
			*/
			cmdBuffer.Begin();
			cmdBuffer.MemoryBarrier(image, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferSrcOptimal, AccessFlag::TransferRead, range);
			cmdBuffer.MemoryBarrier(*destination, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, AccessFlag::TransferWrite);

			/* Copy the actual data and end the buffer. */
			cmdBuffer.CopyEntireImage(image, *destination);
			cmdBuffer.End();

			parent.transferQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		virtual Result Continue(void) override
		{
			/* Get the data from the destination buffer. */
			destination->BeginMemoryTransfer();
			const void *data = destination->GetHostMemory();

			/* Make sure the directory exists. */
			FileWriter::CreateDirectory(path.fileDirectory());

			/* Save the image to disk. */
			int32 result = 0;
			string pathU8 = path.toUTF8();
			const Extent3D extent = image.GetExtent();

			switch (format)
			{
			case Pu::ImageSaveFormats::Png:
				result = stbi_write_png((pathU8 + u8".png").c_str(), extent.Width, extent.Height, image.GetChannels(), data, 0);
				break;
			case Pu::ImageSaveFormats::Bmp:
				result = stbi_write_bmp((pathU8 + u8".bmp").c_str(), extent.Width, extent.Height, image.GetChannels(), data);
				break;
			case Pu::ImageSaveFormats::Tga:
				result = stbi_write_tga((pathU8 + u8".tga").c_str(), extent.Width, extent.Height, image.GetChannels(), data);
				break;
			case Pu::ImageSaveFormats::Jpg:
				result = stbi_write_jpg((pathU8 + u8".jpg").c_str(), extent.Width, extent.Height, image.GetChannels(), data, 100);
				break;
			case Pu::ImageSaveFormats::Hdr:
				result = stbi_write_hdr((pathU8 + u8".hdr").c_str(), extent.Width, extent.Height, image.GetChannels(), reinterpret_cast<const float*>(data));
				break;
			}

			/* Check the result and end the map. */
			if (!result) Log::Error("Unable to save image to '%ls', reason: '%ls'!", path, _CrtGetErrorString().c_str());
			destination->EndMemoryTransfer();
			return Result::AutoDelete();
		}

	protected:
		virtual bool ShouldContinue(void) const override
		{
			/* If the copy is done the command buffer can begin again. */
			return cmdBuffer.CanBegin();
		}

	private:
		AssetSaver &parent;
		const Image &image;
		CommandBuffer &cmdBuffer;
		wstring path;
		ImageSaveFormats format;
		Buffer *destination;
	};

	/* Create the save task. */
	SaveTask *task = new SaveTask(*this, image, path, format);
	scheduler.Spawn(*task);
}

void Pu::AssetSaver::AllocateCmdBuffer(void)
{
	buffers.emplace(std::move(cmdPool->Allocate()));
}

Pu::CommandBuffer & Pu::AssetSaver::GetCmdBuffer(void)
{
	/* Allocate a new command buffer if the queue is empty. */
	if (!buffers.available()) AllocateCmdBuffer();
	return buffers.get();
}