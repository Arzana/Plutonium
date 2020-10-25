#include "Content/AssetLoader.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"
#include "Graphics/Lighting/LightProbeRenderer.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Graphics/Models/ShapeCreator.h"
#include "Core/Diagnostics/Stopwatch.h"
#include "Core/Diagnostics/Profiler.h"
#include "Streams/FileReader.h"

Pu::AssetLoader::AssetLoader(LogicalDevice & device, AssetCache & cache)
	: cache(cache), device(device),
	transferQueue(device.GetTransferQueue(0)), graphicsQueue(device.GetGraphicsQueue(0))
{}

void Pu::AssetLoader::PopulateRenderpass(Renderpass & renderpass, const vector<vector<wstring>> & shaders)
{
	/* The first index is the subpass, the second is the shader and the string is the path. */
	vector<std::tuple<size_t, size_t, wstring>> toLoad;

	size_t i = 0;
	for (const vector<wstring> &subpass : shaders)
	{
		/* Add an empty subpass to the renderpass. */
		renderpass.subpasses.emplace_back();
		size_t j = 0;

		for (const wstring &path : subpass)
		{
			/* Create an unique indentifier for the subpass. */
			const size_t shaderHash = std::hash<wstring>{}(path);

			/* Check if the subpass is already loaded. */
			if (!cache.Reserve(shaderHash))
			{
				renderpass.subpasses.back().shaders.emplace_back(&cache.Get(shaderHash).Duplicate<Shader>(cache));
			}
			else
			{
				/* Create a new asset and store it in the cache, we need to already set the hash because the shader might appear twice in the requested list. */
				Shader *shader = new Shader(device);
				shader->SetHash(shaderHash);
				cache.Store(shader);

				/* Add the subpass to the to-load list and add it to the renderpass. */
				toLoad.emplace_back(i, j, path);
				renderpass.subpasses.back().shaders.emplace_back(shader);
			}

			++j;
		}

		++i;
	}

	/* The load task is deleted by the scheduler as the continue has an auto delete set. */
	Renderpass::LoadTask *task = new Renderpass::LoadTask(renderpass, toLoad);
	TaskScheduler::Force(*task);
}

void Pu::AssetLoader::PopulateComputepass(ShaderProgram & program, const wstring & shader)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(LogicalDevice &device, AssetCache &cache, ShaderProgram &result, const wstring &path)
			: Task("Load Compute Program"), device(device), cache(cache), result(result), path(path)
		{}

		Result Execute(void) final
		{
			/* Check if the shader was already loaded. */
			const size_t shaderHash = std::hash<wstring>{}(path);
			if (!cache.Reserve(shaderHash))
			{
				result.shaders.emplace_back(&cache.Get(shaderHash).Duplicate<Shader>(cache));
				return Result::AutoDelete();
			}

			/* Create the new shader asset and store it. */
			Shader *shader = new Shader(device);
			shader->SetHash(shaderHash);
			cache.Store(shader);

			/* Create the shader load task and make it a child of this task. */
			result.shaders.emplace_back(shader);
			child = new Shader::LoadTask(*shader, path);
			child->SetParent(*this);
			TaskScheduler::Spawn(*child);

			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			/* Parse the required information from the shader and clean up. */
			result.Link(device, true);
			delete child;
			return Result::AutoDelete();
		}

	private:
		LogicalDevice &device;
		AssetCache &cache;
		ShaderProgram &result;
		Shader::LoadTask *child;
		wstring path;
	};

	/* The load task is deleted by the scheduler as the continue has an auto delete set. */
	LoadTask *task = new LoadTask(device, cache, program, shader);
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const wstring & path, const ImageInformation & info)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const ImageInformation &info, const wstring &path)
			: Task("Initialize Texture"), result(texture), parent(parent), staged(false), name(path.fileNameWithoutExtension())
		{
			child = new Texture::LoadTask(texture, info, path);
			child->SetParent(*this);
		}

		Result Execute(void) final
		{
			/* Just execute the texture load task. */
			TaskScheduler::Spawn(*child);
			return Result::Default();
		}

		Result Continue(void) final
		{
			/*
			There are two stages after the texels are loaded.
			First we stage the image data by applying a copy all command to the transfer queue.
			Secondly we delete the staging buffer and mark the texture as loaded.
			*/
			if (staged)
			{
				parent.FinalizeTexture(result, std::move(name));
				delete child;
				return Result::AutoDelete();
			}
			else
			{
				/* We allocate a new command buffer here to put less stress on the caller. */
				cmdBuffer.Initialize(parent.device, parent.transferQueue.GetFamilyIndex());

				/*  We start by staging the image from disk to the firt mip level (0). */
				cmdBuffer.Begin();
				cmdBuffer.MemoryBarrier(result, PipelineStageFlags::TopOfPipe, PipelineStageFlags::Transfer, ImageLayout::TransferDstOptimal, AccessFlags::TransferWrite, ImageSubresourceRange{ ImageAspectFlags::Color });
				cmdBuffer.CopyEntireBuffer(child->GetStagingBuffer(), *result.Image);
				cmdBuffer.End();

				staged = true;
				parent.transferQueue.Submit(cmdBuffer);
				return Result::CustomWait();
			}
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* The texture is done staging if the buffer can begin again. */
			if (staged) return cmdBuffer.CanBegin();
			return Task::ShouldContinue();
		}

	private:
		Texture &result;
		AssetLoader &parent;
		SingleUseCommandBuffer cmdBuffer;
		Texture::LoadTask *child;
		bool staged;
		wstring name;
	};

	/* Simply create the task and spawn it. */
	StageTask *task = new StageTask(*this, texture, info, path);
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const vector<wstring>& paths, const ImageInformation & info, const wstring &name)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const ImageInformation &info, const vector<wstring> &paths, const wstring &name)
			: Task("Initialize Texture Array"), result(texture), parent(parent), staged(false), name(name)
		{
			children.reserve(paths.size());
			for (const wstring &path : paths)
			{
				children.emplace_back(new Texture::LoadTask(texture, info, path));
				children.back()->SetParent(*this);
			}
		}

		Result Execute(void) final
		{
			/* Load all the underlying textures. */
			for (Texture::LoadTask *child : children) TaskScheduler::Spawn(*child);
			return Result::Default();
		}

		Result Continue(void) final
		{
			/*
			There are two stages after the texels are loaded.
			First we stage the image data by applying a copy all command to the transfer queue.
			Secondly we delete the staging buffers and mark the texture as loaded.
			*/
			if (staged)
			{
				parent.FinalizeTexture(result, std::move(name));
				for (const Texture::LoadTask *child : children) delete child;
				return Result::AutoDelete();
			}
			else
			{
				/* We allocate a new command buffer here to put less stress on the caller. */
				cmdBuffer.Initialize(parent.device, parent.transferQueue.GetFamilyIndex());

				/*  Begin the command buffer and add the memory barrier to ensure a good layout. */
				cmdBuffer.Begin();
				ImageSubresourceRange range{ ImageAspectFlags::Color };
				range.LayerCount = static_cast<uint32>(children.size());
				cmdBuffer.MemoryBarrier(result, PipelineStageFlags::TopOfPipe, PipelineStageFlags::Transfer, ImageLayout::TransferDstOptimal, AccessFlags::TransferWrite, range);

				/* Copy actual data and end the buffer. */
				uint32 face = 0;
				for (Texture::LoadTask *child : children)
				{
					/* Each staging buffer hold one face from the cube map. */
					BufferImageCopy region{ result.Image->GetExtent() };
					region.ImageSubresource.BaseArrayLayer = face++;
					cmdBuffer.CopyBuffer(child->GetStagingBuffer(), *result.Image, region);
				}
				cmdBuffer.End();

				staged = true;
				parent.transferQueue.Submit(cmdBuffer);
				return Result::CustomWait();
			}
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* The texture is done staging if the buffer can begin again. */
			if (staged) return cmdBuffer.CanBegin();
			return Task::ShouldContinue();
		}

	private:
		Texture &result;
		AssetLoader &parent;
		SingleUseCommandBuffer cmdBuffer;
		vector<Texture::LoadTask*> children;
		wstring name;
		bool staged;
	};

	/* Simply create the task and spawn it. */
	StageTask *task = new StageTask(*this, texture, info, paths, name);
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const byte * data, size_t size, wstring && id)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const byte *data, size_t size, wstring &&id)
			: Task("Load Raw Texture"), result(texture), parent(parent), id(std::move(id))
		{
			this->data = (byte*)malloc(size);
			memcpy(this->data, data, size);
			stagingBuffer = new StagingBuffer(parent.GetDevice(), size);
		}

		~StageTask()
		{
			delete stagingBuffer;
			free(data);
		}

		Result Execute(void) final
		{
			/* We allocate a new command buffer here to put less stress on the caller. */
			cmdBuffer.Initialize(parent.device, parent.transferQueue.GetFamilyIndex());

			/* Load the image data into the staging buffer. */
			stagingBuffer->Load(data);

			/*  Begin the command buffer and add the memory barrier to ensure a good layout. */
			cmdBuffer.Begin();
			cmdBuffer.MemoryBarrier(result, PipelineStageFlags::TopOfPipe, PipelineStageFlags::Transfer, ImageLayout::TransferDstOptimal, AccessFlags::TransferWrite, result.GetFullRange());

			/* Copy actual data and end the buffer. */
			cmdBuffer.CopyEntireBuffer(*stagingBuffer, *result.Image);
			cmdBuffer.End();

			/* Submit and wait for completion. */
			parent.transferQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			/* We still need to create mipmaps for the texture so finalize it in another task. */
			parent.FinalizeTexture(result, std::move(id));
			return Result::AutoDelete();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* The texture is done staging if the buffer can begin again. */
			return cmdBuffer.CanBegin();
		}

	private:
		Texture &result;
		AssetLoader &parent;
		SingleUseCommandBuffer cmdBuffer;
		StagingBuffer *stagingBuffer;
		byte *data;
		wstring id;
	};

	/* Simply create the task and spawn it. */
	StageTask *task = new StageTask(*this, texture, data, size, std::move(id));
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::FinalizeTexture(Texture & texture, wstring && id)
{
	class FinalizeTask
		: public Task
	{
	public:
		FinalizeTask(AssetLoader &parent, Texture &texture, wstring && id)
			: Task("Generate Mipmaps"), result(texture.GetImage()), parent(parent), name(std::move(id))
		{}

		Result Execute(void) final
		{
			/* We first generate all of the mip levels and then we move the entire image to it's final layout. */
			cmdBuffer.Initialize(parent.device, parent.graphicsQueue.GetFamilyIndex());
			cmdBuffer.Begin();

			for (uint32 arrayLayer = 0; arrayLayer < result.GetArrayLayers(); arrayLayer++)
			{
				/* We need to override out default state after every layer except for the last, sow e just override at the start instead, because it doesn't matter for the first one. */
				result.OverrideState(ImageLayout::TransferDstOptimal, AccessFlags::TransferWrite);

				/* Make sure that the origional mip (level 0) is transitioned to a transfer source. */
				ImageSubresourceRange srcRange{ ImageAspectFlags::Color };
				srcRange.BaseArraylayer = arrayLayer;
				cmdBuffer.MemoryBarrier(result, PipelineStageFlags::Transfer, PipelineStageFlags::Transfer, ImageLayout::TransferSrcOptimal, AccessFlags::TransferRead, srcRange);

				for (uint32 srcLevel = 0, dstLevel = 1; dstLevel < result.GetMipLevels(); srcLevel++, dstLevel++)
				{
					/* We have yet to access this mip level in the image so its layout is undefined. */
					result.OverrideState(ImageLayout::Undefined, AccessFlags::None);

					/* Transition the destination mip level to a transfer destination. */
					ImageSubresourceRange dstRange{ ImageAspectFlags::Color };
					dstRange.BaseArraylayer = arrayLayer;
					dstRange.BaseMipLevel = dstLevel;
					cmdBuffer.MemoryBarrier(result, PipelineStageFlags::Transfer, PipelineStageFlags::Transfer, ImageLayout::TransferDstOptimal, AccessFlags::TransferWrite, dstRange);

					/* Blit the source mip level to the destination mip level, resizing the image by 50%. */
					const ImageBlit blit{ arrayLayer, srcLevel, dstLevel, result.GetExtent().To2D() };
					cmdBuffer.BlitImage(result, ImageLayout::TransferSrcOptimal, result, ImageLayout::TransferDstOptimal, blit, Filter::Linear);

					/* Transition the current mip level to a transfer source so we can use it in our next rotation. */
					cmdBuffer.MemoryBarrier(result, PipelineStageFlags::Transfer, PipelineStageFlags::Transfer, ImageLayout::TransferSrcOptimal, AccessFlags::TransferRead, dstRange);
				}
			}

			/* Transition the entire image from a transfer source to shader read only optimal so it can be used by the user. */
			cmdBuffer.MemoryBarrier(result, PipelineStageFlags::Transfer, PipelineStageFlags::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlags::ShaderRead, result.GetFullRange(ImageAspectFlags::Color));

			/* We need to wait for the queue to process our command before we can mark the image as done, image blitting also needs to be done on a graphics queue. */
			cmdBuffer.End();
			parent.graphicsQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			result.MarkAsLoaded(true, std::move(name));
			return Result::AutoDelete();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* The mipmaps are generated and the final layout is present when the command buffer can begin again. */
			return cmdBuffer.CanBegin();
		}

	private:
		Image &result;
		AssetLoader &parent;
		SingleUseCommandBuffer cmdBuffer;
		wstring name;
	};

	/* Simply create the task and spawn it. */
	FinalizeTask *task = new FinalizeTask(*this, texture, std::move(id));
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::InitializeFont(Font & font, const wstring & path, Task & continuation)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(AssetLoader &parent, Font &font, const wstring &path, Task &continuation)
			: Task("Stage Font"), result(font), parent(parent), path(path), continuation(continuation)
		{}

		Result Execute(void) final
		{
			/* We allocate a new command buffer here to put less stress on the caller. */
			cmdBuffer.Initialize(parent.device, parent.transferQueue.GetFamilyIndex());

			/* Load the glyph information. */
			result.Load(path);
			const Vector2 imgSize = result.LoadGlyphInfo();

			/* Create and populate the staging buffer. */
			const size_t stagingBufferSize = static_cast<size_t>(imgSize.X) * static_cast<size_t>(imgSize.Y);
			buffer = new StagingBuffer(parent.device, stagingBufferSize);
			buffer->BeginMemoryTransfer();
			result.StageAtlas(imgSize, reinterpret_cast<byte*>(buffer->GetHostMemory()));
			buffer->EndMemoryTransfer();

			/* Create the result image. */
			const Extent3D extent(static_cast<uint32>(imgSize.X), static_cast<uint32>(imgSize.Y), 1);
			ImageCreateInfo info(ImageType::Image2D, Format::R8_UNORM, extent, 1, 1, SampleCountFlags::Pixel1Bit, ImageUsageFlags::TransferDst | ImageUsageFlags::Sampled);
			result.atlasImg = new Image(parent.device, info);

			/* Make sure the atlas has the correct layout. */
			cmdBuffer.Begin();
			cmdBuffer.MemoryBarrier(*result.atlasImg,
				PipelineStageFlags::TopOfPipe,
				PipelineStageFlags::Transfer,
				ImageLayout::TransferDstOptimal,
				AccessFlags::TransferWrite,
				result.atlasImg->GetFullRange(ImageAspectFlags::Color));

			/* Copy actual data and end the buffer. */
			cmdBuffer.CopyEntireBuffer(*buffer, *result.atlasImg);
			cmdBuffer.End();

			parent.transferQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			/* Mark both the image and the font as loaded, font will delete the atlas so just mark it as not loaded via the loader. */
			wstring name = L"Atlas ";
			name += path.fileNameWithoutExtension();
			result.atlasImg->MarkAsLoaded(false, std::move(name));

			/* Delete the staging buffer and this task. */
			delete buffer;
			return Result(&continuation, true, false);
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* The texture is done staging if the buffer can begin again. */
			return cmdBuffer.CanBegin();
		}

	private:
		Font &result;
		AssetLoader &parent;
		Task &continuation;
		SingleUseCommandBuffer cmdBuffer;
		const wstring path;
		StagingBuffer *buffer;
	};

	/* Simply create the task and spawn it. */
	LoadTask *task = new LoadTask(*this, font, path, continuation);
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::InitializeModel(Model & model, const wstring & path, const DeferredRenderer & deferred, const LightProbeRenderer * probes)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(AssetLoader &parent, Model &model, const wstring &path, const DeferredRenderer &deferred, const LightProbeRenderer *probes)
			: Task("Load Model"), result(model), parent(parent), path(path), deferred(deferred), probes(probes)
		{}

		Result Execute(void) final
		{
			/* Initialize the model, loading the meshes. */
			PuMData data = PuMData::MeshesOnly(parent.GetDevice(), path);
			result.meshes.Initialize(parent.GetDevice(), data);
			result.nodes = std::move(data.Nodes);

			/* Stage the vertex and index buffer to the GPU on the graphics queue. */
			parent.StageBuffer(*data.Buffer, result.meshes.GetBuffer(), PipelineStageFlags::VertexInput, AccessFlags::VertexAttributeRead, L"GPU Mesh");
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			/* This means that the model initialization is done. */
			if (cmdBuffer.IsInitialized())
			{
				result.MarkAsLoaded(true, path.fileNameWithoutExtension());
				return Result::AutoDelete();
			}

			/* We only need the material information at this stage. */
			const PuMData data = PuMData::MaterialsOnly(path);

			/* Record the descriptor commands. */
			cmdBuffer.Initialize(parent.GetDevice(), parent.graphicsQueue.GetFamilyIndex());
			cmdBuffer.Begin();
			result.Finalize(cmdBuffer, deferred, probes, data);
			cmdBuffer.End();

			parent.graphicsQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* Finalization is done once the command buffer can begin again. */
			if (cmdBuffer.IsInitialized()) return cmdBuffer.CanBegin();

			/*
			We can only finalize the model once:
			- All textures are loaded
			- The GPU data is staged.
			- The deferred and light probe renderer are usable.
			*/
			for (const Texture2D *cur : result.textures)
			{
				if (!cur->IsUsable()) return false;
			}

			if (!deferred.IsUsable()) return false;
			if (probes && !probes->IsUsable()) return false;

			return result.meshes.GetBuffer().IsLoaded();
		}

	private:
		Model &result;
		AssetLoader &parent;
		const wstring path;
		const DeferredRenderer &deferred;
		const LightProbeRenderer *probes;
		SingleUseCommandBuffer cmdBuffer;
	};

	LoadTask *task = new LoadTask(*this, model, path, deferred, probes);
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::CreateModel(Model & model, ShapeType shape, const DeferredRenderer & deferred, const LightProbeRenderer * probes)
{
	constexpr uint16 SPHERE_DIVS = 12;
	constexpr uint16 DOME_DIVS = 24;
	constexpr uint16 TORUS_DIVS = 24;
	constexpr uint16 CYLINDER_DIVS = 24;
	constexpr uint16 CONE_DIVS = 24;

	class CreateTask
		: public Task
	{
	public:
		CreateTask(AssetLoader &parent, Model &model, ShapeType type, const DeferredRenderer &deferred, const LightProbeRenderer *probes)
			: Task("Generate Model"), result(model), parent(parent), deferred(deferred), probes(probes), meshType(type)
		{}

		Result Execute(void) final
		{
			/* Get the required size of the staging buffer. */
			size_t bufferSize = 0;
			uint32 vrtxSize = 0;
			switch (meshType)
			{
			case ShapeType::Plane:
				bufferSize = ShapeCreator::PlaneBufferSize;
				vrtxSize = ShapeCreator::PlaneVertexSize;
				break;
			case ShapeType::Box:
				bufferSize = ShapeCreator::BoxBufferSize;
				vrtxSize = ShapeCreator::BoxVertexSize;
				break;
			case ShapeType::Sphere:
				bufferSize = ShapeCreator::GetSphereBufferSize(SPHERE_DIVS);
				vrtxSize = ShapeCreator::GetSphereVertexSize(SPHERE_DIVS);
				break;
			case ShapeType::Dome:
				bufferSize = ShapeCreator::GetDomeBufferSize(DOME_DIVS);
				vrtxSize = ShapeCreator::GetDomeVertexSize(DOME_DIVS);
				break;
			case ShapeType::Torus:
				bufferSize = ShapeCreator::GetTorusBufferSize(TORUS_DIVS);
				vrtxSize = ShapeCreator::GetTorusVertexSize(TORUS_DIVS);
				break;
			case ShapeType::Cylinder:
				bufferSize = ShapeCreator::GetCylinderBufferSize(CYLINDER_DIVS);
				vrtxSize = ShapeCreator::GetCylinderVertexSize(CYLINDER_DIVS);
				break;
			case ShapeType::Cone:
				bufferSize = ShapeCreator::GetConeBufferSize(CONE_DIVS);
				vrtxSize = ShapeCreator::GetConeVertexSize(CONE_DIVS);
				break;
			default:
				Log::Error("Cannot create mesh from shape type: '%s'!", to_string(meshType));
				return Result::AutoDelete();
			}

			/* Allocate the source and destination buffer. */
			StagingBuffer *src = new StagingBuffer(parent.GetDevice(), bufferSize);

			/* Create the mesh. */
			Mesh mesh;
			switch (meshType)
			{
			case ShapeType::Plane:
				mesh = ShapeCreator::Plane(*src);
				break;
			case ShapeType::Box:
				mesh = ShapeCreator::Box(*src);
				break;
			case ShapeType::Sphere:
				mesh = ShapeCreator::Sphere(*src, SPHERE_DIVS);
				break;
			case ShapeType::Dome:
				mesh = ShapeCreator::Dome(*src, DOME_DIVS);
				break;
			case ShapeType::Torus:
				mesh = ShapeCreator::Torus(*src, TORUS_DIVS, 0.5f);
				break;
			case ShapeType::Cylinder:
				mesh = ShapeCreator::Cylinder(*src, CYLINDER_DIVS);
				break;
			case ShapeType::Cone:
				mesh = ShapeCreator::Cone(*src, CONE_DIVS);
				break;
			}

			/* Add the basic mesh to the model's list. */
			result.meshes.Initialize(parent.GetDevice(), *src, vrtxSize, mesh);
			parent.StageBuffer(*src, result.meshes.GetBuffer(), PipelineStageFlags::VertexInput, AccessFlags::VertexAttributeRead, L"Procedural Mesh");
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			/* This means that the model initialization is done. */
			if (cmdBuffer.IsInitialized())
			{
				result.MarkAsLoaded(true, string(to_string(meshType)).toWide());
				return Result::AutoDelete();
			}

			/* Record the descriptor commands. */
			cmdBuffer.Initialize(parent.GetDevice(), parent.graphicsQueue.GetFamilyIndex());
			cmdBuffer.Begin();
			result.Finalize(cmdBuffer, deferred, probes);
			cmdBuffer.End();

			parent.graphicsQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* Finalization is done once the command buffer can begin again. */
			if (cmdBuffer.IsInitialized()) return cmdBuffer.CanBegin();

			/*
			We can only finalize the model once:
			- All textures are loaded
			- The GPU data is staged.
			- The deferred and light probe renderer are usable.
			*/
			for (const Texture2D *cur : result.textures)
			{
				if (!cur->IsUsable()) return false;
			}

			if (!deferred.IsUsable()) return false;
			if (probes && !probes->IsUsable()) return false;

			return result.meshes.GetBuffer().IsLoaded();
		}

	private:
		Model &result;
		AssetLoader &parent;
		const DeferredRenderer &deferred;
		const LightProbeRenderer *probes;
		SingleUseCommandBuffer cmdBuffer;
		ShapeType meshType;
	};

	CreateTask *task = new CreateTask(*this, model, shape, deferred, probes);
	TaskScheduler::Spawn(*task);
}

void Pu::AssetLoader::StageBuffer(StagingBuffer & source, Buffer & destination, PipelineStageFlags dstStage, AccessFlags access, const wstring & name)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, StagingBuffer &src, Buffer &dst, PipelineStageFlags dstStage, AccessFlags access, const wstring &name)
			: Task("Stage Buffer"), parent(parent), source(&src), destination(dst), dstStage(dstStage), access(access), name(name), timer(parent.GetDevice(), "Staging", Color::Gray())
		{}

		Result Execute(void) final
		{
			/* We can use the faster GPU DMA engine (transfer queue) if this resource is just transfer related. */
			Queue &queue = dstStage == PipelineStageFlags::Transfer ? parent.transferQueue : parent.graphicsQueue;
			cmdBuffer.Initialize(parent.GetDevice(), queue.GetFamilyIndex());

			/* We need to copy the entire staging buffer to the destination buffer and move the destination buffer to the correct access. */
			cmdBuffer.Begin();
			timer.RecordTimestamp(cmdBuffer, 0, PipelineStageFlags::Transfer);
			cmdBuffer.CopyEntireBuffer(*source, destination);
			cmdBuffer.MemoryBarrier(destination, PipelineStageFlags::Transfer, dstStage, access);
			timer.RecordTimestamp(cmdBuffer, 0, PipelineStageFlags::Transfer);
			cmdBuffer.End();

			queue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			delete source;
			destination.MarkAsLoaded(false, std::move(name));

			Profiler::Add(timer, cmdBuffer, false);
			return Result::AutoDelete();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			return cmdBuffer.CanBegin();
		}

	private:
		AssetLoader &parent;
		StagingBuffer *source;
		Buffer &destination;
		SingleUseCommandBuffer cmdBuffer;
		PipelineStageFlags dstStage;
		AccessFlags access;
		wstring name;
		ProfilerChain timer;
	};

	StageTask *task = new StageTask(*this, source, destination, dstStage, access, name);
	TaskScheduler::Spawn(*task);
}