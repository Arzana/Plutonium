#include "Graphics/Vulkan/Pipelines/Pipeline.h"

Pu::Pipeline::Pipeline(Pipeline && value)
	: Hndl(value.Hndl), LayoutHndl(value.LayoutHndl), Device(value.Device),
	specInfos(std::move(value.specInfos)), shaderStages(std::move(value.shaderStages))
{
	value.Hndl = nullptr;
	value.LayoutHndl = nullptr;
}

Pu::Pipeline & Pu::Pipeline::operator=(Pipeline && other)
{
	if (this != &other)
	{
		FullDestroy();

		Hndl = other.Hndl;
		LayoutHndl = other.LayoutHndl;
		Device = other.Device;
		specInfos = std::move(other.specInfos);
		shaderStages = std::move(other.shaderStages);

		other.Hndl = nullptr;
		other.LayoutHndl = nullptr;
	}

	return *this;
}

void Pu::Pipeline::SetSpecializationData(uint32 shader, const void * data, size_t size)
{
#ifdef _DEBUG
	if (shaderStages.size() <= shader) Log::Fatal("Pipeline doesn't contain shader at index %u!", shader);
#endif

	/* Set the data. */
	SpecializationInfo &specInfo = *const_cast<SpecializationInfo*>(shaderStages[shader].SpecializationInfo);
	specInfo.DataSize = static_cast<uint32>(size);
	specInfo.Data = data;
}

Pu::Pipeline::Pipeline(LogicalDevice & device, const Subpass & subpass)
	: Device(&device)
{
	/* Set the shader stage information. */
	size_t bufferCnt = 0;
	shaderStages.reserve(subpass.GetShaders().size());
	for (const Shader *shader : subpass.GetShaders())
	{
		bufferCnt += shader->specializationConstants.size() > 0;
		shaderStages.emplace_back(shader->info);
	}

	/* Match the specilization infos with the matching shader infos. */
	specInfos.resize(bufferCnt);
	for (size_t i = 0, j = 0; i < shaderStages.size(); i++)
	{
		if (subpass.GetShaders()[i]->specializationConstants.empty()) continue;
		shaderStages[i].SpecializationInfo = specInfos.data() + j++;
	}

	/* Create the pipeline layout. */
	CreatePipelineLayout(subpass);
}

void Pu::Pipeline::InitializeSpecializationConstants(const Subpass & subpass)
{
	if (specInfos.empty()) return;

#ifdef _DEBUG
	/* Make sure a data buffer is supplied for every specialization constant. */
	uint32 idx = 0;
	for (const PipelineShaderStageCreateInfo &stageInfo : shaderStages)
	{
		if (stageInfo.SpecializationInfo)
		{
			if (!(stageInfo.SpecializationInfo->Data || stageInfo.SpecializationInfo->DataSize))
			{
				Log::Fatal("No valid specialization constant data was supplied for shader %ls!", subpass[idx].GetName().c_str());
			}
		}

		++idx;
	}
#endif

	/* Make sure the old data is destroyed. */
	DestroyBuffers();

	/* Process the specialization constants. */
	for (size_t i = 0, j = 0, k = 0; i < subpass.GetShaders().size(); i++, k = 0)
	{
		/* Skip any shader that doesn't have any specialization constants. */
		const Shader &shader = subpass[i];
		if (shader.specializationConstants.empty()) continue;

		/* Allocate a heap buffer for the entries so they'll be kept alive. */
		SpecializationInfo &specInfo = specInfos[j++];
		specInfo.MapEntryCount = static_cast<uint32>(shader.specializationConstants.size());
		specInfo.MapEntries = reinterpret_cast<SpecializationMapEntry*>(malloc(sizeof(SpecializationMapEntry) * specInfo.MapEntryCount));

#ifdef _DEBUG
		size_t rangeStart1 = 0, rangeEnd1 = 0;
#endif

		/* Add all the entries to the buffer. */
		for (const SpecializationConstant &specConst : shader.specializationConstants)
		{
			const_cast<SpecializationMapEntry*>(specInfo.MapEntries)[k++] = specConst.entry;

#ifdef _DEBUG
			/* Check whether any of the specified ranges overlap, this should not occur by design, but is possible. */
			const size_t rangeStart2 = specConst.entry.Offset;
			const size_t rangeEnd2 = rangeStart2 + specConst.entry.Size;
			if (max(rangeStart1, rangeStart2) < min(rangeEnd1, rangeEnd2))
			{
				Log::Warning("Specialization constant entry %s in shader %ls is overlapping with another specialization constant!", specConst.name.c_str(), shader.GetName().c_str());
			}

			rangeStart1 = min(rangeStart1, rangeStart2);
			rangeEnd1 = max(rangeEnd1, rangeEnd2);
#endif
		}
	}
}

void Pu::Pipeline::Destroy(void)
{
	if (Hndl) Device->vkDestroyPipeline(Device->hndl, Hndl, nullptr);

#ifdef _DEBUG
	for (SpecializationInfo &cur : specInfos)
	{
		cur.Data = nullptr;
		cur.DataSize = 0;
	}
#endif
}

void Pu::Pipeline::CreatePipelineLayout(const Subpass & subpass)
{
	/* Scan the subpass for push constant ranges. */
	vector<PushConstantRange> pushRanges;
	for (const PushConstant &pushConstant : subpass.pushConstants)
	{
		/* Add the size of the push constant if we find multiple in the same shader stage. */
		bool add = true;
		for (PushConstantRange &range : pushRanges)
		{
			if (range.StageFlags == pushConstant.range.StageFlags)
			{
				range.Size += static_cast<uint32>(pushConstant.GetSize());
				add = false;
				break;
			}
		}

		/* Otherwise add a new range to the list. */
		if (add) pushRanges.emplace_back(pushConstant.range);
	}

	/* Just get a list of the descriptor set Vulkan handles. */
	vector<DescriptorSetHndl> hndls;
	hndls.reserve(subpass.setLayouts.size());
	for (const DescriptorSetLayout &layout : subpass.setLayouts) hndls.emplace_back(layout.hndl);

	/* Create the pipeline layout. */
	const PipelineLayoutCreateInfo createInfo{ hndls, pushRanges };
	VK_VALIDATE(Device->vkCreatePipelineLayout(Device->hndl, &createInfo, nullptr, &LayoutHndl), PFN_vkCreatePipelineLayout);
}

void Pu::Pipeline::DestroyBuffers(void)
{
	for (SpecializationInfo &cur : specInfos)
	{
		if (cur.MapEntryCount)
		{
			free(const_cast<SpecializationMapEntry*>(cur.MapEntries));
			cur.MapEntryCount = 0;
			cur.MapEntries = nullptr;
		}
	}
}

void Pu::Pipeline::FullDestroy(void)
{
	/*
	First destroy the actual pipeline.
	After that; destroy the layouts and buffers.
	*/
	Destroy();
	DestroyBuffers();
	if (LayoutHndl) Device->vkDestroyPipelineLayout(Device->hndl, LayoutHndl, nullptr);
}