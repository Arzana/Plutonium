#include "Graphics/Vulkan/DescriptorSetGroup.h"

Pu::DescriptorSetGroup::DescriptorSetGroup(DescriptorPool & pool)
	: DescriptorSetBase(pool), subscribe(false)
{}

Pu::DescriptorSetGroup::DescriptorSetGroup(DescriptorSetGroup && value)
	: DescriptorSetBase(std::move(value)), subscribe(value.subscribe), hndls(std::move(value.hndls))
{
	if (subscribe) Pool->OnStage.Add(*this, &DescriptorSetGroup::Stage);
}

Pu::DescriptorSetGroup & Pu::DescriptorSetGroup::operator=(DescriptorSetGroup && other)
{
	if (this != &other)
	{
		DescriptorSetBase::operator=(std::move(other));
		Free();

		subscribe = other.subscribe;
		hndls = std::move(other.hndls);

		if (subscribe) Pool->OnStage.Add(*this, &DescriptorSetGroup::Stage);
	}

	return *this;
}

Pu::DeviceSize Pu::DescriptorSetGroup::Add(uint32 subpass, const DescriptorSetLayout & layout)
{
	
	const uint64 id = Pool->MakeId(subpass, layout.GetSet());

	/* Check for invalid usage on debug mode. */
#ifdef _DEBUG
	if (hndls.find(id) != hndls.end()) Log::Fatal("Attempting to add already added subpass set to descriptor set group!");
#endif

	/* Subscribe to the pools stage event if we need to. */
	if (layout.HasUniformBufferMemory() && !subscribe)
	{
		subscribe = true;
		Pool->OnStage.Add(*this, &DescriptorSetGroup::Stage);
	}

	/* Allocate the set and add it to the list. */
	DescriptorSetHndl hndl;
	const DeviceSize offset = Pool->Alloc(subpass, layout, &hndl);
	hndls.emplace(id, hndl);

	return offset;
}

void Pu::DescriptorSetGroup::Write(uint32 subpass, const Descriptor & descriptor, const TextureInput & input)
{
	/* Get the handle to the descriptor set. */
	const uint64 id = Pool->MakeId(subpass, descriptor.GetSet());
	decltype(hndls)::const_iterator it = hndls.find(id);

	/* Check for invalid usage on debug mode. */
#ifdef _DEBUG
	if (it == hndls.end())
	{
		Log::Fatal("Attempting to write input attachment to unknown descriptor set in descriptor set group!");
		return;
	}
#endif

	DescriptorSetBase::Write(it->second, descriptor.GetSet(), descriptor, input);
}

void Pu::DescriptorSetGroup::Write(uint32 subpass, const Descriptor & descriptor, const Texture & texture)
{
	/* Get the handle to the descriptor set. */
	const uint64 id = Pool->MakeId(subpass, descriptor.GetSet());
	decltype(hndls)::const_iterator it = hndls.find(id);

	/* Check for invalid usage on debug mode. */
#ifdef _DEBUG
	if (it == hndls.end())
	{
		Log::Fatal("Attempting to write texture attachment to unknown descriptor set in descriptor set group!");
		return;
	}
#endif

	DescriptorSetBase::Write(it->second, descriptor.GetSet(), descriptor, texture);
}

void Pu::DescriptorSetGroup::Free(void)
{
	/* Free all the underlying handles. */
	for (const auto [id, hndl] : hndls) Pool->Free(hndl);
	hndls.clear();

	if (subscribe)
	{
		subscribe = false;
		Pool->OnStage.Remove(*this, &DescriptorSetGroup::Stage);
	}
}