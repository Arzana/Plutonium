#include "Graphics/Vulkan/QueryPool.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::QueryPool::QueryPool(LogicalDevice & device, QueryType type, size_t count)
	: parent(&device), count(static_cast<uint32>(count))
{
	const QueryPoolCreateInfo createInfo(type, static_cast<uint32>(count));
	VK_VALIDATE(parent->vkCreateQueryPool(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateQueryPool);
}

Pu::QueryPool::QueryPool(LogicalDevice & device, size_t count, QueryPipelineStatisticFlags statistics)
	: parent(&device), count(static_cast<uint32>(count))
{
	const QueryPoolCreateInfo createInfo(QueryType::PipelineStatistics, static_cast<uint32>(count), statistics);
	VK_VALIDATE(parent->vkCreateQueryPool(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateQueryPool);
}

Pu::QueryPool::QueryPool(QueryPool && value)
	: parent(value.parent), hndl(value.hndl), count(value.count)
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
		count = other.count;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::vector<Pu::uint32> Pu::QueryPool::GetResults(uint32 firstQuery, uint32 resultCount, bool wait, bool partial) const
{
	/* Check for if we're not accessing outsize of pool range on debug. */
#ifdef _DEBUG
	CheckRange(firstQuery, 1);
#endif

	/* Define the flags. */
	QueryResultFlags flags = QueryResultFlags::None;
	if (wait) flags |= QueryResultFlags::Wait;
	if (partial) flags |= QueryResultFlags::Partial;

	/* Query the result. */
	vector<uint32> results(resultCount);
	const VkApiResult result = parent->vkGetQueryPoolResults(parent->hndl, hndl, firstQuery, 1, resultCount * sizeof(uint32), results.data(), sizeof(uint32), flags);

	/* If the result is not ready, just resturn an empty result. */
	if (result == VkApiResult::NotReady) return vector<uint32>();
	return results;
}

float Pu::QueryPool::GetTimeDelta(uint32 firstQuery, bool wait) const
{
	float result = 0.0f;
	if (GetTimeDeltaInternal(firstQuery, wait ? QueryResultFlags::Wait : QueryResultFlags::None, result)) return result;
	return 0.0f;
}

bool Pu::QueryPool::TryGetTimeDelta(uint32 firstQuery, float & result) const
{
	return GetTimeDeltaInternal(firstQuery, QueryResultFlags::None, result);
}

Pu::uint32 Pu::QueryPool::GetOcclusion(uint32 queryIndex, bool wait) const
{
	uint32 result = 0;
	if (GetOcclusionInternal(queryIndex, wait ? QueryResultFlags::Wait : QueryResultFlags::None, result)) return result;
	return 0;
}

bool Pu::QueryPool::TryGetOcclusion(uint32 queryIndex, uint32 & result) const
{
	return GetOcclusionInternal(queryIndex, QueryResultFlags::None, result);
}

bool Pu::QueryPool::GetOcclusionInternal(uint32 firstQuery, QueryResultFlags flag, uint32 & value) const
{
	/* Check for if we're not accessing outsize of pool range on debug. */
#ifdef _DEBUG
	CheckRange(firstQuery, 1);
#endif

	/* Gets the result. */
	const VkApiResult result = parent->vkGetQueryPoolResults(parent->hndl, hndl, firstQuery, 1, sizeof(uint32), &value, sizeof(uint32), flag);
	return result != VkApiResult::NotReady;
}

bool Pu::QueryPool::GetTimeDeltaInternal(uint32 firstQuery, QueryResultFlags flag, float & delta) const
{
	/* Check for if we're not accessing outsize of pool range on debug. */
#ifdef _DEBUG
	CheckRange(firstQuery, 2);
#endif

	/* Get the number of nanoseconds requireed for a timestamp to be incremented by 1. */
	const float period = parent->GetPhysicalDevice().GetLimits().TimestampPeriod;

	/* Query the two timestamps. */
	uint32 timestamps[2];
	const VkApiResult result = parent->vkGetQueryPoolResults(parent->hndl, hndl, firstQuery, 2, sizeof(timestamps), timestamps, sizeof(uint32), flag);

	/* Set the delta and return whether the result is valid. */
	delta = (timestamps[1] - timestamps[0]) * period;
	return result != VkApiResult::NotReady;
}

void Pu::QueryPool::CheckRange(uint32 start, uint32 cnt) const
{
	if (start + cnt > count) Log::Fatal("Attempting to access query outside of buffer range (%u > %u)!", start + cnt, count);
}

void Pu::QueryPool::Destroy(void)
{
	if (hndl) parent->vkDestroyQueryPool(parent->hndl, hndl, nullptr);
}