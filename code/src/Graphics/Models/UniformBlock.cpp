#include "Graphics/Models/UniformBlock.h"

Pu::UniformBlock::UniformBlock(const GraphicsPipeline & pipeline, std::initializer_list<string> uniforms)
	: DescriptorSet(std::move(pipeline.GetDescriptorPool().Allocate(CheckAndGetSet(pipeline, uniforms)))),
	IsDirty(false), firstUpdate(true)
{
	/* Calculate the size based on the uniforms. */
	vector<uint32> knownBindings;
	size_t size = 0;
	for (const string &name : uniforms)
	{
		/* We can only add them here; not in the CheckAndGetSet because that has to be called in the initializer list. */
		const Descriptor &descriptor = pipeline.GetRenderpass().GetDescriptor(name);
		this->descriptors.emplace_back(&descriptor);

		/* If the binding already exists we just append, but if it doesn't exist we need to take the GPU allignment into account. */
		if (knownBindings.contains(descriptor.GetBinding())) size += descriptor.GetSize();
		else
		{
			knownBindings.emplace_back(descriptor.GetBinding());
			size = descriptor.GetAllignedOffset(size) + descriptor.GetSize();
		}
	}

	target = new DynamicBuffer(pipeline.GetDevice(), size, BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst);
}

Pu::UniformBlock::UniformBlock(UniformBlock && value)
	: DescriptorSet(std::move(value)), descriptors(std::move(value.descriptors)),
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
		descriptors = std::move(other.descriptors);
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
			Write(descriptors, *target);
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
		const Descriptor &uniform = pipeline.GetRenderpass().GetDescriptor(*it);

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