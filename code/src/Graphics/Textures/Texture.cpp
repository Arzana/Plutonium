#include "Graphics/Textures/Texture.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::Texture::Texture(Texture && value)
	: Image(value.Image), Sampler(value.Sampler), 
	view(value.view), refCnt(value.refCnt)
{
	value.view = nullptr;
}

Pu::Texture & Pu::Texture::operator=(Texture && other)
{
	if (this != &other)
	{
		Destroy();

		Image = other.Image;
		Sampler = other.Sampler;
		view = other.view;
		refCnt = other.refCnt;

		other.view = nullptr;
	}

	return *this;
}

Pu::Texture::Texture(Pu::Sampler * sampler, Pu::Image & image, ImageViewType type)
	: Image(&image), Sampler(sampler), refCnt(1)
{
	view = type != ImageViewType::None ? new ImageView(*this, type, ImageAspectFlag::Color) : nullptr;
}

void Pu::Texture::Reference(void)
{
	if (Sampler) Sampler->Reference();
	Image->Reference();
	++refCnt;
}

void Pu::Texture::Destroy(void)
{
	if (view) delete view;
}

Pu::Texture::LoadTask::LoadTask(Texture & result, const ImageInformation & info, const wstring & path)
	: result(result), info(info), child(nullptr), stagingBuffer(nullptr), path(path)
{
	/* Create the child task as either a HDR load or LDR load. */
	if (info.IsHDR) child = new ImageLoadTask<float>(path);
	else child = new ImageLoadTask<byte>(path);

	child->SetParent(*this);
}

Pu::Texture::LoadTask::~LoadTask(void)
{
	if (stagingBuffer) delete stagingBuffer;
}

Pu::Task::Result Pu::Texture::LoadTask::Execute(void)
{
	/* Start loading the image. */
	scheduler->Spawn(*child);
	return Result::Default();
}

Pu::Task::Result Pu::Texture::LoadTask::Continue(void)
{
	if (info.IsHDR)
	{
		/* Get the result from the load task. */
		const ImageLoadTask<float> *loadTask = static_cast<ImageLoadTask<float>*>(child);
		const vector<float> &texels = loadTask->GetData();

		/* Copy texel information to staging buffer. */
		stagingBuffer = new StagingBuffer(result.Image->GetDevice(), texels.size() * sizeof(float));
		stagingBuffer->Load(texels.data());

		delete loadTask;
	}
	else
	{
		/* Get the result from the load task. */
		const ImageLoadTask<byte> *loadTask = static_cast<ImageLoadTask<byte>*>(child);
		const vector<byte> &texels = loadTask->GetData();

		/* Copy texel information to staging buffer. */
		stagingBuffer = new StagingBuffer(result.Image->GetDevice(), texels.size() * sizeof(byte));
		stagingBuffer->Load(texels.data());

		delete loadTask;
	}

	return Result::Default();
}