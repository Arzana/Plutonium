#include "Graphics/Diagnostics/QueryChain.h"

inline constexpr Pu::byte queryCountPerType(Pu::QueryType type)
{
	return 1 + (type == Pu::QueryType::Timestamp);
}

constexpr Pu::byte MASK_RESET = 0x7F;
constexpr Pu::byte MASK_SET = 0x80;

Pu::QueryChain::QueryChain(LogicalDevice & device, QueryType type, size_t chainCount, size_t bufferCount)
	: QueryPool(device, type, queryCountPerType(type) * bufferCount * chainCount), stats(QueryPipelineStatisticFlag::None)
{
	/* Check for invalid arguments on debug mode. */
#ifdef _DEBUG
	logged = false;
	if (chainCount < 1) Log::Fatal("Chain count must be greater than 0!");
	if (bufferCount < 1) Log::Fatal("Buffer count must be greater than 0!");
#endif

	/* Allocate the chain buffer. */
	chains.reserve(chainCount);
	for (size_t i = 0; i < chainCount; i++) chains.emplace_back(type, bufferCount, i);
}

Pu::QueryChain::QueryChain(LogicalDevice & device, QueryPipelineStatisticFlag statistics, size_t bufferCount)
	: QueryPool(device, bufferCount, statistics), stats(statistics)
{
	/* Check for invalid arguments on debug mode. */
#ifdef _DEBUG
	logged = false;
	if (statistics == QueryPipelineStatisticFlag::None) Log::Fatal("No flags were set for pipeline statistics!");
	if (bufferCount < 1) Log::Fatal("Buffer count must be greater than 0!");
#endif

	/* A pipeline statistics query chain only has one chain link. */
	chains.emplace_back(QueryType::PipelineStatistics, bufferCount, 0);
}

void Pu::QueryChain::Reset(CommandBuffer & cmdBuffer)
{
	for (const Chain &chain : chains)
	{
		const uint32 start = chain.GetNext();
		cmdBuffer.ResetQueries(*this, start, chain.flags & MASK_RESET);
	}
}

void Pu::QueryChain::RecordTimestamp(CommandBuffer & cmdBuffer, uint32 chain, PipelineStageFlag stage)
{
	Chain &ref = chains[chain];

	/* Get the index of the current set of queries. */
	const uint32 i = ref.GetNext();

	/* Use the correct index for the query. */
	if (ref.flags & MASK_SET)
	{
		ref.flags &= ~MASK_SET;

		/* Add the timestamp to the chain if it was the second one. */
		ref.chain.emplace(i);
		cmdBuffer.WriteTimestamp(stage, *this, i + 1);
	}
	else
	{
		ref.flags |= MASK_SET;
		cmdBuffer.WriteTimestamp(stage, *this, i);
	}
}

void Pu::QueryChain::RecordStatistics(CommandBuffer & cmdBuffer)
{
	/* Gets the pipeline chain and the current index. */
	Chain &ref = chains.front();
	const uint32 i = ref.GetNext();

	if (ref.flags & MASK_SET)
	{
		ref.flags &= ~MASK_SET;

		/* End the statistics query if this was the second call. */
		cmdBuffer.EndQuery(*this, i - 1);
	}
	else
	{
		/* Begin the statistics query if this was the first call. */
		ref.flags |= MASK_SET;
		ref.chain.emplace(i);
		cmdBuffer.BeginQuery(*this, i);
	}
}

float Pu::QueryChain::GetTimeDelta(uint32 chain) const
{
	Chain &ref = chains[chain];

	/* Check for valid use on debug. */
#ifdef _DEBUG
	if (ref.flags & MASK_SET) Log::Fatal("Only one of the two timestamps needed to get the time delta has been set!");
#endif

	/* Early out if nothing has been recorded yet. */
	if (ref.chain.size())
	{
		/* Get the first index in our queue, this is the one we'll try to get. */
		const uint32 i = ref.chain.front();
		float result = 0.0f;

		if (TryGetTimeDelta(i, result))
		{
			/* Remove the old index from our queue if we've got it and return the result. */
			ref.chain.pop();
			return result;
		}
	}

	return 0.0f;
}

Pu::vector<Pu::uint32> Pu::QueryChain::GetStatistics(void) const
{
	vector<uint32> result;

	Chain &ref = chains.front();
	if (ref.chain.size())
	{
		result = GetResults(ref.chain.front(), _mm_popcnt_u32(_CrtEnum2Int(stats)), false, false);
		ref.chain.pop();
	}

	return result;
}

const char * Pu::QueryChain::GetStatisticName(uint32 idx) const
{
	/* Loop through all the bits until we find the set bit at the user specified index. */
	for (uint32 i = 0, j = 0; i < sizeof(uint32) << 3; i++)
	{
		const uint32 mask = 1 << i;
		const bool set = _CrtEnum2Int(stats) & mask;

		if (set && j == idx) return to_string(_CrtInt2Enum<QueryPipelineStatisticFlag>(mask));
		j += set;
	}

	return "Unknown";
}

Pu::QueryChain::Chain::Chain(QueryType type, size_t bufferCount, size_t idx)
	: flags(queryCountPerType(type))
{
	first = static_cast<uint32>(idx * bufferCount * flags);
	last = first + static_cast<uint32>(bufferCount * flags);
}

Pu::uint32 Pu::QueryChain::Chain::GetNext(void) const
{
	if (chain.empty()) return first;

	/* We use the chain as a ring buffer, so modulo back to zero once we reach the end. */
	const uint32 result = chain.back() + (flags & MASK_RESET);
	return result >= last ? first + result % last : result;
}