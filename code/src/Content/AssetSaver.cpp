#include "Content/AssetSaver.h"
#include "Streams/FileWriter.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"

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
	: scheduler(scheduler), device(device), OnAssetSaved("OnAssetSaved"),
	graphicsQueue(device.GetGraphicsQueue(0))
{}

void Pu::AssetSaver::SaveImage(const Image & image, const wstring & path, ImageSaveFormats format)
{
	class SaveTask
		: public Task
	{
	public:
		SaveTask(AssetSaver &parent, const Image &image, const wstring &path, ImageSaveFormats format)
			: parent(parent), image(image), path(path), format(format)
		{}

		virtual Result Execute(void) override
		{
			/* Create the command buffer, it's better for the caller to do it here than the ctor. */
			cmdBuffer.Initialize(parent.device, parent.graphicsQueue.GetFamilyIndex());

			/* Creat the buffer needed as a result for the image data. */
			const Extent3D extent = image.GetExtent();
			const size_t imageSizeBytes = extent.Width * extent.Height * extent.Depth * image.GetElementSize();
			destination = new Buffer(parent.device, imageSizeBytes, BufferUsageFlag::TransferDst, MemoryPropertyFlag::HostVisible);

			/*
			Begin the command buffer and ensure a usable layouts.
			We can only start this the first memory barrier at transfer because the image source access is probably transfer destination.
			The buffer on the other hand has no access flag set so we can do it at the top of pipe already.
			*/
			cmdBuffer.Begin();
			cmdBuffer.MemoryBarrier(image, PipelineStageFlag::FragmentShader, PipelineStageFlag::Transfer, ImageLayout::TransferSrcOptimal, AccessFlag::TransferRead, image.GetFullRange(ImageAspectFlag::Color));
			cmdBuffer.MemoryBarrier(*destination, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, AccessFlag::TransferWrite);

			/* Copy the actual data and end the buffer. */
			cmdBuffer.CopyEntireImage(image, *destination);
			cmdBuffer.End();

			/* We need to submit on the graphics queue as the origional image access fill most likely be shader read. */
			parent.graphicsQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		virtual Result Continue(void) override
		{
			/* Get the data from the destination buffer. */
			destination->BeginMemoryTransfer();
			const void *data = destination->GetHostMemory();

			/* Save the image to disk. */
			const Extent3D extent = image.GetExtent();
			const int w = static_cast<int>(extent.Width), h = static_cast<int>(extent.Height), c = static_cast<int>(image.GetChannels());
			AssetSaver::SaveImageInternal(data, w, h, c, path, format);

			/* Finalize the result. */
			destination->EndMemoryTransfer();
			delete destination;

			/* Make sure to post the event just before deleting the task. */
			parent.OnAssetSaved.Post(parent, image);
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
		SingleUseCommandBuffer cmdBuffer;
		wstring path;
		ImageSaveFormats format;
		Buffer *destination;
	};

	/* Create the save task. */
	SaveTask *task = new SaveTask(*this, image, path, format);
	scheduler.Spawn(*task);
}

void Pu::AssetSaver::SaveImage(const void * data, uint32 width, uint32 height, Format format, const wstring & path, ImageSaveFormats saveFormat)
{
	SaveImageInternal(data, static_cast<int>(width), static_cast<int>(height), static_cast<int>(format_channels(format)), path, saveFormat);
}

void Pu::AssetSaver::SaveImageInternal(const void * data, int w, int h, int c, const wstring & path, ImageSaveFormats format)
{
	/* Make sure the directory exists (only if there is a directory in the path of course). */
	const wstring dir = path.fileDirectory();
	if (!dir.empty()) FileWriter::CreateDirectory(dir);

	/* Save the image to disk. */
	int32 result = 0;
	string pathU8 = path.toUTF8();

	switch (format)
	{
	case Pu::ImageSaveFormats::Png:
		result = stbi_write_png((pathU8 += u8".png").c_str(), w, h, c, data, 0);
		break;
	case Pu::ImageSaveFormats::Bmp:
		result = stbi_write_bmp((pathU8 += u8".bmp").c_str(), w, h, c, data);
		break;
	case Pu::ImageSaveFormats::Tga:
		result = stbi_write_tga((pathU8 += u8".tga").c_str(), w, h, c, data);
		break;
	case Pu::ImageSaveFormats::Jpg:
		result = stbi_write_jpg((pathU8 += u8".jpg").c_str(), w, h, c, data, 100);
		break;
	case Pu::ImageSaveFormats::Hdr:
		result = stbi_write_hdr((pathU8 += u8".hdr").c_str(), w, h, c, reinterpret_cast<const float*>(data));
		break;
	}

	/* Check the result and end the map. */
	if (result) Log::Verbose("Successfully saved image '%ls'.", path.fileName().c_str());
	else Log::Error("Unable to save image to '%ls', reason: '%ls'!", path.c_str(), _CrtGetErrorString().c_str());
}