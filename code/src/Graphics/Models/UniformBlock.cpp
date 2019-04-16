#include "Graphics/Models/UniformBlock.h"

Pu::UniformBlock::UniformBlock(LogicalDevice & device, const GraphicsPipeline & pipeline, std::initializer_list<string> uniforms)
	: DescriptorSet(std::move(pipeline.GetDescriptorPool().Allocate(CheckAndGetSet(pipeline, uniforms)))),
	IsDirty(false), firstUpdate(true)
{
	/* Calculate the size based on the uniforms. */
	vector<uint32> knownBindings;
	size_t size = 0;
	for (const string &name : uniforms)
	{
		/* We can only add them here; not in the CheckAndGetSet because that has to be called in the initializer list. */
		const Uniform &uniform = pipeline.GetRenderpass().GetUniform(name);
		this->uniforms.emplace_back(&uniform);

		/* If the binding already exists we just append, but if it doesn't exist we need to take the GPU allignment into account. */
		if (knownBindings.contains(uniform.GetBinding())) size += uniform.GetSize();
		else
		{
			knownBindings.emplace_back(uniform.GetBinding());
			size = uniform.GetAllignedOffset(size) + uniform.GetSize();
		}
	}

	target = new DynamicBuffer(device, size, BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst);
}

Pu::UniformBlock::UniformBlock(UniformBlock && value)
	: DescriptorSet(std::move(value)), uniforms(std::move(value.uniforms)),
	IsDirty(value.IsDirty), firstUpdate(value.firstUpdate)
{
	target = value.target;
	value.target = nullptr;
}

Pu::UniformBlock & Pu::UniformBlock::operator=(UniformBlock && other)
{
	if (this != &other)
	{
		Destroy();

		DescriptorSet::operator=(std::move(other));
		uniforms = std::move(other.uniforms);
		IsDirty = other.IsDirty;
		firstUpdate = other.firstUpdate;
		target = other.target;

		other.target = nullptr;
	}

	return *this;
}

void Pu::UniformBlock::Update(CommandBuffer & cmdBuffer)
{
	/* Only update the GPU data if needed. */
	if (IsDirty)
	{
		/* Ask the derived uniform block to upload its data to the staging buffer. */
		target->BeginMemoryTransfer();
		Stage(reinterpret_cast<byte*>(target->GetHostMemory()));
		target->EndMemoryTransfer();

		/* Copy the staging buffer into the uniform buffer. */
		target->Update(cmdBuffer);

		/* 
		The descriptor only needs to be set once and the memory barrier is only needed the first time.
		This is because the buffer will remain in uniform read mode throughout the program (after the first update),
		and the descriptor just keeps pointing to the same resource.
		*/
		if (firstUpdate)
		{
			cmdBuffer.MemoryBarrier(*target, PipelineStageFlag::Transfer, PipelineStageFlag::VertexShader, AccessFlag::UniformRead);
			Write(uniforms, *target);
			firstUpdate = false;
		}

		IsDirty = false;
	}
}

/* uniforms hides class member. */
#pragma warning(push)
#pragma warning(disable:4458)
Pu::uint32 Pu::UniformBlock::CheckAndGetSet(const GraphicsPipeline & pipeline, std::initializer_list<string> uniforms)
{
	uint32 result = 0;

	for (std::initializer_list<string>::const_iterator it = uniforms.begin(); it != uniforms.end(); it++)
	{
		const Uniform &uniform = pipeline.GetRenderpass().GetUniform(*it);

		/* Make sure all the descriptors are from the same set. */
		if (it != uniforms.begin())
		{
			if (uniform.GetSet() != result) Log::Fatal("Cannot pass uniforms from different sets to a uniform block!");
		}
		else result = uniform.GetSet();
	}

	return result;
}
#pragma warning(pop)

void Pu::UniformBlock::Destroy(void)
{
	if (target) delete target;
}