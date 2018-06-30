#pragma once
#include "Counter.h"
#include "Graphics\Diagnostics\DeviceInfo.h"

namespace Plutonium
{
	/* Provides a simple GPU RAM counter. */
	struct VRamCounter
		: Counter<uint64, uint64>
	{
	public:
		/* Initializes a new instance of a RAM counter. */
		VRamCounter(_In_ Game *game, _In_opt_ size_t bufferSize = 100, _In_opt_ float rate = 0.1f);
		VRamCounter(_In_ const VRamCounter &value) = delete;
		VRamCounter(_In_ VRamCounter &&value) = delete;

		_Check_return_ VRamCounter& operator =(_In_ const VRamCounter &other) = delete;
		_Check_return_ VRamCounter& operator =(_In_ VRamCounter &&other) = delete;

		/* Gets the total physical VRAM from the GPU. */
		_Check_return_ inline uint64 GetOSBudget(void) const
		{
			return budget;
		}

	protected:
		/* Sets value to to the result of memory stats. */
		_Check_return_ virtual bool TryAppendBufferOnUpdate(_In_ float dt, _Out_ uint64 *value) const
		{
			*value = _CrtGetUsedGPUMemory();
			return true;
		}

		/* Returns false. */
		_Check_return_ virtual bool TryAppendBUfferOnDraw(_In_ float dt, _Out_ uint64 *value) const
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
				uint64 cur = buffer.at(i);

				sum += cur;
				if (*worst < cur) *worst = cur;
				if (*best > cur) *best = cur;
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

	private:
		uint64 budget;
	};
}