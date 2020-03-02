#include "Graphics/Diagnostics/QueryChain.h"

inline constexpr Pu::byte queryCountPerType(Pu::QueryType type)
{
	return type == Pu::QueryType::Occlusion ? 1 : 2;
}

constexpr Pu::byte MASK_RESET = 0x7F;
constexpr Pu::byte MASK_SET = 0x80;

Pu::QueryChain::QueryChain(LogicalDevice & device, QueryType type, size_t bufferCount)
	: QueryPool(device, type, queryCountPerType(type) * bufferCount), 
	flags(queryCountPerType(type))
#ifdef _DEBUG
	, logged(false)
#endif
{
	/* Check for invalid arguments on debug mode. */
#ifdef _DEBUG
	if (bufferCount < 1) Log::Fatal("Buffer count must be greater than 0!");
#endif
}

void Pu::QueryChain::Reset(CommandBuffer & cmdBuffer)
{
	if (chain.empty()) return;

	const uint32 start = GetNext();
	cmdBuffer.ResetQueries(*this, start, flags & MASK_RESET);
}

void Pu::QueryChain::RecordTimestamp(CommandBuffer & cmdBuffer, PipelineStageFlag stage)
{
	/* Get the index of the current set of queries. */
	const uint32 i = chain.empty() ? 0 : GetNext();

	/* Use the correct index for the query. */
	if (flags & MASK_SET)
	{
		flags &= ~MASK_SET;

		/* Add the timestamp to the chain if it was the second one. */
		chain.emplace(i);
		cmdBuffer.WriteTimestamp(stage, *this, i + 1);
	}
	else
	{
		flags |= MASK_SET;
		cmdBuffer.WriteTimestamp(stage, *this, i);
	}
}

float Pu::QueryChain::GetTimeDelta(void) const
{
	/* Check for valid use on debug. */
#ifdef _DEBUG
	if (flags & MASK_SET) Log::Fatal("Only one of the two timestamps needed to get the time delta has been set!");
#endif

	/* Early out if nothing has been recorded yet. */
	if (chain.size())
	{
		/* Get the first index in our queue, this is the one we'll try to get. */
		const uint32 i = chain.front();
		float result = 0.0f;

		if (TryGetTimeDelta(i, result))
		{
			/* Remove the old index from our queue if we've got it and return the result. */
			chain.pop();
			return result;
		}
	}

	return 0.0f;
}

Pu::uint32 Pu::QueryChain::GetNext(void) const
{
	/* We use the chain as a circular buffer, so modulo loop back to the start. */
	return (chain.back() + (flags & MASK_RESET)) % GetPoolSize();
}
