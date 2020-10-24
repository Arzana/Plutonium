#include "Graphics/Vulkan/Pipelines/Pipeline.h"

Pu::Pipeline::Pipeline(Pipeline && value)
	: Hndl(value.Hndl), LayoutHndl(value.LayoutHndl), Device(value.Device),
	specInfos(std::move(value.specInfos)), shaderStages(std::move(value.shaderStages)),
	CreateFlags(value.CreateFlags)
#ifdef _DEBUG
	, exeProperties(std::move(value.exeProperties)), stats(std::move(value.stats)),
	internals(std::move(value.internals))
#endif
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
		CreateFlags = other.CreateFlags;
		specInfos = std::move(other.specInfos);
		shaderStages = std::move(other.shaderStages);

#ifdef _DEBUG
		exeProperties = std::move(other.exeProperties);
		stats = std::move(other.stats);
		internals = std::move(other.internals);
#endif

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

bool Pu::Pipeline::EnableDebugging(void)
{
#ifdef _DEBUG
	if (Hndl) Log::Fatal("Cannot call EnableDebugging after the pipeline is finalized!");
#else
	Log::Warning("Pipeline debugging is only available on debug mode!");
	return false;
#endif

	if (Device->GetPhysicalDevice().executablePropertiesSupported)
	{
		/* Allows the use of GetStatistics and GetInternals. */
		CreateFlags |= PipelineCreateFlags::CaptureStatistics | PipelineCreateFlags::CaptureInternalRepresentations;
		return true;
	}

	return false;
}

const Pu::vector<Pu::PipelineExecutableProperties>& Pu::Pipeline::GetExecutableProperties(void) const
{
#ifdef _DEBUG
	if (!Hndl) Log::Fatal("Pipeline executable properties can only be queried after the pipeline is finalized!");
	InitExeProps();
	return exeProperties;
#else
	Log::Fatal("Pipeline debugging is only available on debug mode!");
	return vector<PipelineExecutableProperties>();
#endif
}

const Pu::vector<Pu::PipelineExecutableStatistic>& Pu::Pipeline::GetExecutableStatistics(uint32 executableIndex) const
{
#ifdef _DEBUG
	/* Check for invalid use. */
	if (!Hndl) Log::Fatal("Pipeline statistics can only be queried after the pipeline is finalized!");
	if (!_CrtEnumCheckFlag(CreateFlags, PipelineCreateFlags::CaptureStatistics)) Log::Fatal("Debugging must be enabled in order to query pipeline statistics!");

	/* Initialize the global executable properties and check if the user argument is valid. */
	InitExeProps();
	if (executableIndex >= exeProperties.size()) Log::Fatal("Pipeline doesn't have an executable at index %u!", executableIndex);

	/* Use the cache if available. */
	decltype(stats)::const_iterator it = stats.find(executableIndex);
	if (it != stats.end()) return it->second;

	/* Get the amount of statistics for this executable. */
	const PipelineExecutableInfo info{ Hndl, executableIndex };
	uint32 statCount;
	Device->vkGetPipelineExecutableStatisticsKHR(Device->hndl, &info, &statCount, nullptr);

	/* Query the actual statistics and add it to the cache. */
	vector<PipelineExecutableStatistic> result{ statCount };
	Device->vkGetPipelineExecutableStatisticsKHR(Device->hndl, &info, &statCount, result.data());
	return stats.emplace(executableIndex, std::move(result)).first->second;
#else
	(void)executableIndex;
	Log::Fatal("Pipeline debugging is only available on debug mode!");
	return vector<PipelineExecutableStatistic>();
#endif
}

const Pu::vector<Pu::PipelineExecutableInternalRepresentation> & Pu::Pipeline::GetExecutableInternals(uint32 executableIndex) const
{
#ifdef _DEBUG
	/* Check for invalid use. */
	if (!Hndl) Log::Fatal("Pipeline executable internals can only be queried after the pipeline is finalized!");
	if (!_CrtEnumCheckFlag(CreateFlags, PipelineCreateFlags::CaptureInternalRepresentations)) Log::Fatal("Debugging must be enabled in order to query pipeline executable internals!");

	/* Initialize the global executable properties and check if the user argument is valid. */
	InitExeProps();
	if (executableIndex >= exeProperties.size()) Log::Fatal("Pipeline doesn't have an executable at index %u!", executableIndex);

	/* Use the cache if available. */
	decltype(internals)::const_iterator it = internals.find(executableIndex);
	if (it != internals.end()) return it->second;

	/* Get the amount of internals for this executable. */
	const PipelineExecutableInfo info{ Hndl, executableIndex };
	uint32 internalsCount;
	Device->vkGetPipelineExecutableInternalRepresentationsKHR(Device->hndl, &info, &internalsCount, nullptr);

	/* Query the actual internals and add it to the cache. */
	vector<PipelineExecutableInternalRepresentation> result{ internalsCount };
	Device->vkGetPipelineExecutableInternalRepresentationsKHR(Device->hndl, &info, &internalsCount, result.data());
	return internals.emplace(executableIndex, std::move(result)).first->second;
#else
	(void)executableIndex;
	Log::Fatal("Pipeline debugging is only available on debug mode!");
	return vector<PipelineExecutableInternalRepresentation>();
#endif
}

Pu::Pipeline::Pipeline(LogicalDevice & device, const ShaderProgram & program)
	: Device(&device), CreateFlags(PipelineCreateFlags::None)
{
	/* Set the shader stage information. */
	size_t bufferCnt = 0;
	shaderStages.reserve(program.shaders.size());
	for (const Shader *shader : program.shaders)
	{
		bufferCnt += shader->specializationConstants.size() > 0;
		shaderStages.emplace_back(shader->info);
	}

	/* Match the specilization infos with the matching shader infos. */
	specInfos.resize(bufferCnt);
	for (size_t i = 0, j = 0; i < shaderStages.size(); i++)
	{
		if (program.shaders[i]->specializationConstants.empty()) continue;
		shaderStages[i].SpecializationInfo = specInfos.data() + j++;
	}

	/* Create the pipeline layout. */
	CreatePipelineLayout(program);
}

void Pu::Pipeline::InitializeSpecializationConstants(const ShaderProgram & program)
{
	if (specInfos.empty()) return;

#ifdef _DEBUG
	/* Make sure none of the constant buffer are set to invalid values. */
	uint32 idx = 0;
	for (const PipelineShaderStageCreateInfo &stageInfo : shaderStages)
	{
		if (stageInfo.SpecializationInfo)
		{
			if ((stageInfo.SpecializationInfo->Data != nullptr) ^ (stageInfo.SpecializationInfo->DataSize != 0))
			{
				Log::Error("Specialization constant data for shader %ls was not initialized properly!", program.shaders[idx]->GetName().c_str());
			}
		}

		++idx;
	}
#endif

	/* Make sure the old data is destroyed. */
	DestroyBuffers();

	/* Process the specialization constants. */
	for (size_t i = 0, j = 0, k = 0; i < program.shaders.size(); i++, k = 0)
	{
		/* Skip any shader that doesn't have any specialization constants. */
		const Shader &shader = *program.shaders[i];
		if (shader.specializationConstants.empty()) continue;

		/* Just don't add a map entry for specialization constants left default. */
		SpecializationInfo &specInfo = specInfos[j++];
		if (!(specInfo.Data && specInfo.DataSize))  continue;

		/* Allocate a heap buffer for the entries so they'll be kept alive. */
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
	/* Clear the debugging caches on debug mode. */
	exeProperties.clear();
	stats.clear();
	internals.clear();
#endif
}

#ifdef _DEBUG
void Pu::Pipeline::InitExeProps(void) const
{
	/* We cache the properties, so use that cache if available. */
	if (exeProperties.empty())
	{
		const PipelineInfo info{ Hndl };

		/* Query the amount of executables available for debugging. */
		uint32 propertiesCount;
		Device->vkGetPipelineExecutablePropertiesKHR(Device->hndl, &info, &propertiesCount, nullptr);

		/* Query the actual properties. */
		exeProperties.resize(propertiesCount);
		Device->vkGetPipelineExecutablePropertiesKHR(Device->hndl, &info, &propertiesCount, exeProperties.data());
	}
}
#endif

void Pu::Pipeline::CreatePipelineLayout(const ShaderProgram & program)
{
	/* Scan the subpass for push constant ranges. */
	vector<PushConstantRange> pushRanges;
	for (const PushConstant &pushConstant : program.pushConstants)
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
	hndls.reserve(program.setLayouts.size());
	for (const DescriptorSetLayout &layout : program.setLayouts) hndls.emplace_back(layout.hndl);

	/* Create the pipeline layout. */
	const PipelineLayoutCreateInfo createInfo{ hndls, pushRanges };
	VK_VALIDATE(Device->vkCreatePipelineLayout(Device->hndl, &createInfo, nullptr, &LayoutHndl), PFN_vkCreatePipelineLayout);

	/* Checks whether the pipeline can actually bind all required descriptor sets. */
	if (hndls.size() > Device->parent->GetLimits().MaxBoundDescriptorSets)
	{
		Log::Error("Pipeline 0x%P cannot bind all descriptor sets at once (%zu > %u)!", hndls.size(), Device->parent->GetLimits().MaxBoundDescriptorSets);
	}
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