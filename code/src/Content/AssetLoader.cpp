#include "Content/AssetLoader.h"
#include "Graphics/Vulkan/Shaders/Shader.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"
#include "Streams/FileReader.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/Lighting/LightProbeRenderer.h"
#include "Graphics/Models/ShapeCreator.h"

Pu::AssetLoader::AssetLoader(TaskScheduler & scheduler, LogicalDevice & device, AssetCache & cache)
	: cache(cache), scheduler(scheduler), device(device),
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
			if (cache.Contains(shaderHash))
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
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const wstring & path, const ImageInformation & info)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const ImageInformation &info, const wstring &path)
			: result(texture), parent(parent), staged(false), name(path.fileNameWithoutExtension())
		{
			child = new Texture::LoadTask(texture, info, path);
			child->SetParent(*this);
		}

		Result Execute(void) final
		{
			/* Just execute the texture load task. */
			scheduler->Spawn(*child);
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
				cmdBuffer.MemoryBarrier(result, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, ImageSubresourceRange{ ImageAspectFlag::Color });
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
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const vector<wstring>& paths, const ImageInformation & info, const wstring &name)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const ImageInformation &info, const vector<wstring> &paths, const wstring &name)
			: result(texture), parent(parent), staged(false), name(name)
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
			/* Load all the 6 underlying textures. */
			for (Texture::LoadTask *child : children) scheduler->Spawn(*child);
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
				ImageSubresourceRange range{ ImageAspectFlag::Color };
				range.LayerCount = 6;
				cmdBuffer.MemoryBarrier(result, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, range);

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
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const byte * data, size_t size, wstring && id)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const byte *data, size_t size, wstring &&id)
			: result(texture), parent(parent), id(std::move(id))
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
			cmdBuffer.MemoryBarrier(result, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, result.GetFullRange());

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
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::FinalizeTexture(Texture & texture, wstring && id)
{
	class FinalizeTask
		: public Task
	{
	public:
		FinalizeTask(AssetLoader &parent, Texture &texture, wstring && id)
			: result(texture.GetImage()), parent(parent), name(std::move(id))
		{}

		Result Execute(void) final
		{
			/* We first generate all of the mip levels and then we move the entire image to it's final layout. */
			cmdBuffer.Initialize(parent.device, parent.graphicsQueue.GetFamilyIndex());
			cmdBuffer.Begin();

			for (uint32 arrayLayer = 0; arrayLayer < result.GetArrayLayers(); arrayLayer++)
			{
				/* We need to override out default state after every layer except for the last, sow e just override at the start instead, because it doesn't matter for the first one. */
				result.OverrideState(ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite);

				/* Make sure that the origional mip (level 0) is transitioned to a transfer source. */
				ImageSubresourceRange srcRange{ ImageAspectFlag::Color };
				srcRange.BaseArraylayer = arrayLayer;
				cmdBuffer.MemoryBarrier(result, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferSrcOptimal, AccessFlag::TransferRead, srcRange);

				for (uint32 srcLevel = 0, dstLevel = 1; dstLevel < result.GetMipLevels(); srcLevel++, dstLevel++)
				{
					/* We have yet to access this mip level in the image so its layout is undefined. */
					result.OverrideState(ImageLayout::Undefined, AccessFlag::None);

					/* Transition the destination mip level to a transfer destination. */
					ImageSubresourceRange dstRange{ ImageAspectFlag::Color };
					dstRange.BaseArraylayer = arrayLayer;
					dstRange.BaseMipLevel = dstLevel;
					cmdBuffer.MemoryBarrier(result, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, dstRange);

					/* Blit the source mip level to the destination mip level, resizing the image by 50%. */
					const ImageBlit blit{ arrayLayer, srcLevel, dstLevel, result.GetExtent().To2D() };
					cmdBuffer.BlitImage(result, ImageLayout::TransferSrcOptimal, result, ImageLayout::TransferDstOptimal, blit, Filter::Linear);

					/* Transition the current mip level to a transfer source so we can use it in our next rotation. */
					cmdBuffer.MemoryBarrier(result, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferSrcOptimal, AccessFlag::TransferRead, dstRange);
				}
			}

			/* Transition the entire image from a transfer source to shader read only optimal so it can be used by the user. */
			cmdBuffer.MemoryBarrier(result, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, result.GetFullRange(ImageAspectFlag::Color));

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
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::InitializeFont(Font & font, const wstring & path, Task & continuation)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(AssetLoader &parent, Font &font, const wstring &path, Task &continuation)
			: result(font), parent(parent), path(path), continuation(continuation)
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
			ImageCreateInfo info(ImageType::Image2D, Format::R8_UNORM, extent, 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled);
			result.atlasImg = new Image(parent.device, info);

			/* Make sure the atlas has the correct layout. */
			cmdBuffer.Begin();
			cmdBuffer.MemoryBarrier(*result.atlasImg,
				PipelineStageFlag::TopOfPipe,
				PipelineStageFlag::Transfer,
				ImageLayout::TransferDstOptimal,
				AccessFlag::TransferWrite,
				result.atlasImg->GetFullRange(ImageAspectFlag::Color));

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
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::InitializeModel(Model & model, const wstring & path, const DeferredRenderer & deferred, const LightProbeRenderer & probes)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(AssetLoader &parent, Model &model, const wstring &path, const DeferredRenderer &deferred, const LightProbeRenderer &probes)
			: result(model), parent(parent), path(path), deferred(deferred), probes(probes)
		{}

		Result Execute(void) final
		{
			/* Initialize the model, loading the meshes. */
			const PuMData data = PuMData::MeshesOnly(parent.GetDevice(), path);
			result.Initialize(parent.GetDevice(), data);

			/* Stage the vertex and index buffer to the GPU on the graphics queue. */
			parent.StageBuffer(*data.Buffer, *result.gpuData, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);
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
			if (!probes.IsUsable()) return false;

			return result.gpuData->IsLoaded();
		}

	private:
		Model &result;
		AssetLoader &parent;
		const wstring path;
		const DeferredRenderer &deferred;
		const LightProbeRenderer &probes;
		SingleUseCommandBuffer cmdBuffer;
	};

	LoadTask *task = new LoadTask(*this, model, path, deferred, probes);
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::CreateModel(Model & model, ShapeType shape, const DeferredRenderer & deferred, const LightProbeRenderer & probes)
{
	constexpr uint16 divisions = 12;

	class CreateTask
		: public Task
	{
	public:
		CreateTask(AssetLoader &parent, Model &model, ShapeType type, const DeferredRenderer &deferred, const LightProbeRenderer &probes)
			: result(model), parent(parent), deferred(deferred), probes(probes), meshType(type)
		{}

		Result Execute(void) final
		{
			/* Get the required size of the staging buffer. */
			size_t bufferSize = 0;
			switch (meshType)
			{
			case ShapeType::Plane:
				bufferSize = ShapeCreator::PlaneBufferSize;
				break;
			case ShapeType::Box:
				bufferSize = ShapeCreator::BoxBufferSize;
				break;
			case ShapeType::Sphere:
				bufferSize = ShapeCreator::GetSphereBufferSize(divisions);
				break;
			case ShapeType::Dome:
				bufferSize = ShapeCreator::GetDomeBufferSize(divisions);
				break;
			default:
				Log::Error("Cannot create mesh from shape type: '%s'!", to_string(meshType));
				return Result::AutoDelete();
			}

			/* Allocate the source and destination buffer. */
			StagingBuffer *src = new StagingBuffer(parent.GetDevice(), bufferSize);
			result.AllocBuffer(parent.GetDevice(), *src);

			/* Create the mesh. */
			Mesh mesh;
			switch (meshType)
			{
			case ShapeType::Plane:
				mesh = std::move(ShapeCreator::Plane(*src, *result.gpuData));
				break;
			case ShapeType::Box:
				mesh = std::move(ShapeCreator::Box(*src, *result.gpuData));
				break;
			case ShapeType::Sphere:
				mesh = std::move(ShapeCreator::Sphere(*src, *result.gpuData, divisions));
				break;
			case ShapeType::Dome:
				mesh = std::move(ShapeCreator::Dome(*src, *result.gpuData, divisions));
				break;
			}

			/* Add the basic mesh to the model's list. */
			parent.StageBuffer(*src, *result.gpuData, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);
			result.BasicMeshes.emplace_back(std::make_pair(0, std::move(mesh)));
			result.CalculateBoundingBox();
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

			/* Allocate the single material and create the material. */
			result.AllocPools(deferred, probes, 1);
			result.AddMaterial(0, 1, Model::DefaultMaterialIdx, deferred, probes).SetParameters(1.0f, 2.0f, Color::Black(), Color::White(), 1.0f);

			/* Record the descriptor commands. */
			cmdBuffer.Initialize(parent.GetDevice(), parent.graphicsQueue.GetFamilyIndex());
			cmdBuffer.Begin();
			result.poolMaterials->Update(cmdBuffer, PipelineStageFlag::FragmentShader);
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
			if (!probes.IsUsable()) return false;

			return result.gpuData->IsLoaded();
		}

	private:
		Model &result;
		AssetLoader &parent;
		const DeferredRenderer &deferred;
		const LightProbeRenderer &probes;
		SingleUseCommandBuffer cmdBuffer;
		ShapeType meshType;
	};

	CreateTask *task = new CreateTask(*this, model, shape, deferred, probes);
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::StageBuffer(StagingBuffer & source, Buffer & destination, PipelineStageFlag dstStage, AccessFlag access)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, StagingBuffer &src, Buffer &dst, PipelineStageFlag dstStage, AccessFlag access)
			: parent(parent), source(&src), destination(dst), dstStage(dstStage), access(access)
		{}

		Result Execute(void) final
		{
			cmdBuffer.Initialize(parent.GetDevice(), parent.graphicsQueue.GetFamilyIndex());

			/* We need to copy the entire staging buffer to the destination buffer and move the destination buffer to the correct access. */
			cmdBuffer.Begin();
			cmdBuffer.CopyEntireBuffer(*source, destination);
			cmdBuffer.MemoryBarrier(destination, PipelineStageFlag::Transfer, dstStage, access);
			cmdBuffer.End();

			/* The access might be graphics related, so perform this action on the graphics queue. */
			parent.graphicsQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			delete source;
			destination.MarkAsLoaded(false, L"GPU Buffer");
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
		PipelineStageFlag dstStage;
		AccessFlag access;
	};

	StageTask *task = new StageTask(*this, source, destination, dstStage, access);
	scheduler.Spawn(*task);
}