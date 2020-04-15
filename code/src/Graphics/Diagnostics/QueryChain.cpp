#include "Graphics/Diagnostics/QueryChain.h"

inline constexpr Pu::byte queryCountPerType(Pu::QueryType type)
{
	return type == Pu::QueryType::Occlusion ? 1 : 2;
}

constexpr Pu::byte MASK_RESET = 0x7F;
constexpr Pu::byte MASK_SET = 0x80;

Pu::QueryChain::QueryChain(LogicalDevice & device, QueryType type, size_t chainCount, size_t bufferCount)
	: QueryPool(device, type, queryCountPerType(type) * bufferCount * chainCount)
#ifdef _DEBUG
	, logged(false)
#endif
{
	/* Allocate the chain buffer. */
	chains.reserve(chainCount);
	for (size_t i = 0; i < chainCount; i++) chains.emplace_back(type, bufferCount, i);

	/* Check for invalid arguments on debug mode. */
#ifdef _DEBUG
	if (chainCount < 1) Log::Fatal("Chain count must be greater than 0!");
	if (bufferCount < 1) Log::Fatal("Buffer count must be greater than 0!");
#endif
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