#pragma once
#include "Counter.h"
#include "Core\Diagnostics\Memory.h"

namespace Plutonium
{
	/* Provides a simple CPU RAM counter. */
	class RamCounter
		: public Counter<MemoryFrame, uint64>
	{
	public:
		/* Initializes a new instance of a RAM counter. */
		RamCounter(_In_ Game *game, _In_opt_ size_t bufferSize = 100, _In_opt_ float rate = 0.1f)
			: Counter(game, bufferSize, rate)
		{}

		RamCounter(_In_ const RamCounter &value) = delete;
		RamCounter(_In_ RamCounter &&value) = delete;

		_Check_return_ RamCounter& operator =(_In_ const RamCounter &other) = delete;
		_Check_return_ RamCounter& operator =(_In_ RamCounter &&other) = delete;

		/* Gets the total physical RAM from the OS. */
		_Check_return_ inline uint64 GetOSBudget(void) const
		{
			return buffer.size() > 0 ? buffer.at(buffer.size() - 1).TotalRam : 0UL;
		}

	protected:
		/* Sets value to to the result of memory stats. */
		_Check_return_ virtual bool TryAppendBufferOnUpdate(_In_ float dt, _Out_ MemoryFrame *value) const
		{
			*value = _CrtGetMemStats();
			return true;
		}

		/* Returns false. */
		_Check_return_ virtual bool TryAppendBUfferOnDraw(_In_ float dt, _Out_ MemoryFrame *value) const
		{
			return false;
		}

		/* Updates the display values. */
		virtual void CalculateDisplayValues(_Out_ uint64 *avrg, _Out_ uint64 *worst, _Out_ uint64 *best) const
		{
			/* Reset values and create temporary sum. */
			uint64 sum = 0UL;
			*worst = maxv<uint64>();
			*best = 0UL;

			/* Update sum, worst and best. */
			for (size_t i = 0; i < buffer.size(); i++)
			{
				MemoryFrame cur = buffer.at(i);

				sum += cur.UsedRam;
				if (*worst < cur.UsedRam) *worst = cur.UsedRam;
				if (*best > cur.UsedRam) *best = cur.UsedRam;
			}

			/* Set average. */
			*avrg = sum / buffer.size();
		}

		/* Updates the exclusive display values. */
		virtual void CalculateExclusives(_Inout_ uint64 *exclusiveWorst, _Inout_ uint64 *exclusiveBest, _In_ const uint64 *lastWorst, _In_ const uint64 *lastBest) const
		{
			if (*exclusiveWorst < *lastWorst) *exclusiveWorst = *lastWorst;
			if (*exclusiveBest > *lastBest) *exclusiveBest = *lastBest;
		}
	};
}