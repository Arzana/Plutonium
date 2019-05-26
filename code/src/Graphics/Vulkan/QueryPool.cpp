#include "Graphics/Vulkan/QueryPool.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::QueryPool::QueryPool(LogicalDevice & device, QueryType type, size_t count)
	: parent(&device)
{
	const QueryPoolCreateInfo createInfo(type, static_cast<uint32>(count));
	VK_VALIDATE(parent->vkCreateQueryPool(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateQueryPool);
}

Pu::QueryPool::QueryPool(LogicalDevice & device, size_t count, QueryPipelineStatisticFlag statistics)
	: parent(&device)
{
	const QueryPoolCreateInfo createInfo(QueryType::PipelineStatistics, static_cast<uint32>(count), statistics);
	VK_VALIDATE(parent->vkCreateQueryPool(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateQueryPool);
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

		parent = other.parent;
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
	const VkApiResult result = parent->vkGetQueryPoolResults(parent->hndl, hndl, firstQuery, queryCount, queryCount * sizeof(uint32), results.data(), sizeof(uint32), flags);

	/* If the result is not ready, just resturn an empty result. */
	if (result == VkApiResult::NotReady) return vector<uint32>();
	return results;
}

float Pu::QueryPool::GetTimeDelta(uint32 firstQuery, bool wait)
{
	/* Get the number of nanoseconds requireed for a timestamp to be incremented by 1. */
	const float period = parent->GetPhysicalDevice().GetLimits().TimestampPeriod;

	/* Query the two timestamps. */
	uint32 timestamps[2];
	const VkApiResult result = parent->vkGetQueryPoolResults(parent->hndl, hndl, firstQuery, 2, sizeof(timestamps), timestamps, sizeof(uint32), wait ? QueryResultFlag::Wait : QueryResultFlag::None);

	/* Only return if we could get the timestamps, otherwise just return zero. */
	if (result == VkApiResult::NotReady) return 0.0f;
	else return (timestamps[1] - timestamps[0]) * period;
}

void Pu::QueryPool::Destroy(void)
{
	if (hndl) parent->vkDestroyQueryPool(parent->hndl, hndl, nullptr);
}