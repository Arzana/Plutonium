#include "Graphics/Vulkan/QueryPool.h"

Pu::QueryPool::QueryPool(LogicalDevice & device, QueryType type, size_t count)
	: parent(device)
{
	const QueryPoolCreateInfo createInfo(type, count);
	VK_VALIDATE(parent.vkCreateQueryPool(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateQueryPool);
}

Pu::QueryPool::QueryPool(LogicalDevice & device, size_t count, QueryPipelineStatisticFlag statistics)
	: parent(device)
{
	const QueryPoolCreateInfo createInfo(QueryType::PipelineStatistics, count, statistics);
	VK_VALIDATE(parent.vkCreateQueryPool(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateQueryPool);
}

Pu::QueryPool::QueryPool(QueryPool && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::QueryPool & Pu::QueryPool::operator=(QueryPool && other)
{
	if (this != &other)
	{
		Destroy();

		parent = std::move(other.parent);
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::vector<Pu::uint32> Pu::QueryPool::GetResults(uint32 firstQuery, uint32 queryCount, bool wait, bool partial) const
{
	/* Define the flags. */
	QueryResultFlag flags = QueryResultFlag::None;
	if (wait) flags |= QueryResultFlag::Wait;
	if (partial) flags |= QueryResultFlag::Partial;

	/* Query the result. */
	vector<uint32> results(queryCount);
	const VkApiResult result = parent.vkGetQueryPoolResults(parent.hndl, hndl, firstQuery, queryCount, queryCount * sizeof(uint32), results.data(), sizeof(uint32), flags);

	/* If the result is not ready, just resturn an empty result. */
	if (result == VkApiResult::NotReady) return vector<uint32>();
	return results;
}

void Pu::QueryPool::Destroy(void)
{
	if (hndl) parent.vkDestroyQueryPool(parent.hndl, hndl, nullptr);
}