#include "Graphics/Lighting/LightProbe.h"
#include "Graphics/Lighting/LightProbeRenderer.h"
#include "Graphics/Lighting/LightProbeUniformBlock.h"

Pu::LightProbe::LightProbe(void)
	: fetcher(nullptr), time(0.0f), locked(true),
	near(0.1f), far(100.0f), interval(0.0f), depth(nullptr),
	image(nullptr), view(nullptr), sampler(nullptr), texture(nullptr),
	block(nullptr), framebuffer(nullptr), renderer(nullptr)
{}

Pu::LightProbe::LightProbe(AssetFetcher & fetcher, TextureCube & baked)
	: fetcher(&fetcher), cycleMode(CycleMode::Baked), time(1.0f),
	near(0.1f), far(100.0f), interval(0.0f), depth(nullptr),
	image(nullptr), view(nullptr), sampler(nullptr), texture(&baked),
	block(nullptr), framebuffer(nullptr), renderer(nullptr)
{}

Pu::LightProbe::LightProbe(LightProbe && value)
	: fetcher(value.fetcher), cycleMode(value.cycleMode), time(0.0f),
	near(value.near), far(value.far), interval(value.interval), depth(value.depth),
	image(value.image), view(value.view), sampler(value.sampler), texture(value.texture),
	block(value.block), framebuffer(value.framebuffer), renderer(value.renderer)
{
	if (value.locked.load()) Log::Fatal("Cannot move light probe that is being updated!");

	value.image = nullptr;
	value.view = nullptr;
	value.sampler = nullptr;
	value.texture = nullptr;
	value.block = nullptr;
	value.framebuffer = nullptr;
	value.depth = nullptr;

	CalculateFrustums();
}

Pu::LightProbe::LightProbe(LightProbeRenderer & renderer, Extent2D resolution)
	: fetcher(renderer.loader), cycleMode(CycleMode::OnCommand), time(0.0f),
	near(0.1f), far(100.0f), interval(1.0f), locked(true), renderer(&renderer)
{
	/* The output image is just a 2D cube map image, but we render to it like it's a 2D texture array, so we only have to do 1 draw call per object. */
	ImageCreateInfo imgCreateInfo{ ImageType::Image2D, Format::R8G8B8A8_SRGB, Extent3D(resolution, 1), 1, 6, SampleCountFlag::Pixel1Bit, ImageUsageFlag::Sampled | ImageUsageFlag::ColorAttachment };
	imgCreateInfo.Flags = ImageCreateFlag::CubeCompatible;
	image = new Image(fetcher->GetDevice(), imgCreateInfo);
	view = new ImageView(*image, ImageViewType::Image2DArray, ImageAspectFlag::Color);

	/* The sampler used for the output image as color attachment is just the default sampler. */
	sampler = &fetcher->FetchSampler(SamplerCreateInfo{});
	texture = new TextureCube(*image, *sampler);
	depth = new DepthBuffer(fetcher->GetDevice(), Format::D16_UNORM, resolution, 6);

	/* We need to set the frambuffer and descriptor set, but we can only do that once the renderpass is ready for use. */
	if (renderer.IsUsable()) OnRenderpassDone(*renderer.renderpass);
	else renderer.renderpass->PostCreate.Add(*this, &LightProbe::OnRenderpassDone);
}

Pu::LightProbe & Pu::LightProbe::operator=(LightProbe && other)
{
	if (this != &other)
	{
		if (locked.load() || other.locked.load()) Log::Fatal("Cannot move light probe that is being updated!");

		Destroy();

		fetcher = other.fetcher;
		near = other.near;
		far = other.far;
		interval = other.interval;
		time = other.time;
		cycleMode = other.cycleMode;
		image = other.image;
		view = other.view;
		sampler = other.sampler;
		texture = other.texture;
		block = other.block;
		framebuffer = other.framebuffer;
		renderer = other.renderer;
		depth = other.depth;

		other.image = nullptr;
		other.view = nullptr;
		other.sampler = nullptr;
		other.texture = nullptr;
		other.block = nullptr;
		other.framebuffer = nullptr;
		other.depth = nullptr;

		CalculateFrustums();
	}

	return *this;
}

void Pu::LightProbe::SetPosition(Vector3 value)
{
	position = value;
	if (IsUsable()) CalculateFrustums();
}

/* near, far and interval hide class members. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::LightProbe::SetRange(float near, float far)
{
	this->near = near;
	this->far = far;
	if (IsUsable()) CalculateFrustums();
}

void Pu::LightProbe::SetCycle(CycleMode mode, float interval)
{
	/* Baked light probes never cycle so we ignore setting the cycle interval for that case. */
	if (cycleMode == CycleMode::Baked)
	{
		Log::Error("Cannot override cycle mode of baked light probe!");
		return;
	}

	cycleMode = mode;
	this->interval = interval;
}
#pragma warning(pop)

void Pu::LightProbe::ForceUpdate(void)
{
	if (cycleMode != CycleMode::Baked) time = interval;
}

bool Pu::LightProbe::ShouldUpdate(float dt)
{
	/* Only add time to the counter if the cycle mode is interval. */
	time += dt * (cycleMode == CycleMode::Interval);

	if (time >= interval)
	{
		time = 0.0f;
		return true;
	}

	return false;
}

bool Pu::LightProbe::Cull(const AABB & bb) const
{
	/* The light probe is omnidirectional so we only care about the distance between the object and the light probe. */
	const float d = bb.GetDistance(position);
	return d < near || d > far;
}

Pu::Viewport Pu::LightProbe::GetViewport(void) const
{
	const Image &img = texture->GetImage();
	return Viewport(static_cast<float>(img.GetWidth()), static_cast<float>(img.GetHeight()));
}

void Pu::LightProbe::OnRenderpassDone(Renderpass&)
{
	/* We need a uniform block if we want to render to the probe, we also need to create a layered framebuffer. */
	block = new LightProbeUniformBlock(*renderer->pool);
	framebuffer = new Framebuffer(*renderer->renderpass, image->GetExtent().To2D(), 6, { view, &depth->GetView() });
	CalculateFrustums();
}

void Pu::LightProbe::CalculateFrustums(void)
{
	/* Baked light probes don't need to render. */
	if (cycleMode == CycleMode::Baked) return;

	/* Update all the matrices. */
	const Matrix proj = Matrix::CreatePerspective(PI2, 1.0f, near, far);
	const Matrix translation = Matrix::CreateTranslation(position);

	block->SetFrustum(0, proj * Matrix::CreateLookIn(position, Vector3::Right(), Vector3::Down()));
	block->SetFrustum(1, proj * Matrix::CreateLookIn(position, Vector3::Left(), Vector3::Down()));
	block->SetFrustum(2, proj * Matrix::CreateLookIn(position, Vector3::Down(), Vector3::Forward()));
	block->SetFrustum(3, proj * Matrix::CreateLookIn(position, Vector3::Up(), Vector3::Backward()));
	block->SetFrustum(4, proj * Matrix::CreateLookIn(position, Vector3::Forward(), Vector3::Down()));
	block->SetFrustum(5, proj * Matrix::CreateLookIn(position, Vector3::Backward(), Vector3::Down()));
}

void Pu::LightProbe::Destroy(void)
{
	if (sampler) fetcher->Release(*sampler);
	if (texture)
	{
		/* The texture is loaded through the fetcher if it was a baked light probe. */
		if (cycleMode == CycleMode::Baked) fetcher->Release(*texture);
		else delete texture;
	}

	if (framebuffer) delete framebuffer;
	if (view) delete view;
	if (image) delete image;
	if (block) delete block;
	if (depth) delete depth;
}