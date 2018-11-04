#pragma once
#include "Counter.h"

namespace Plutonium
{
	/* Provides a simple fps counter. */
	class FpsCounter
		: public Counter<float, float>
	{
	public:
		/* Initializes a new instance of an fps counter. */
		FpsCounter(_In_ Game *game, _In_opt_ size_t bufferSize = 100, _In_opt_ float rate = 1.0f)
			: Counter(game, bufferSize, rate)
		{}

		FpsCounter(_In_ const FpsCounter &value) = delete;
		FpsCounter(_In_ FpsCounter &&value) = delete;

		_Check_return_ FpsCounter& operator =(_In_ const FpsCounter &other) = delete;
		_Check_return_ FpsCounter& operator =(_In_ FpsCounter &&other) = delete;

		/* Gets the current draw rate in Hz. */
		_Check_return_ inline float GetCurrentHz(void) const
		{
			return 1.0f / GetCurrent();
		}

		/* Gets the average draw rate in Hz. */
		_Check_return_ inline float GetAverageHz(void) const
		{
			return 1.0f / GetAverage();
		}

		/* Gets the worst stored draw rate in Hz. */
		_Check_return_ inline float GetStoredWorstHz(void) const
		{
			return 1.0f / GetStoredWorst();
		}

		/* Gets the best stored draw rate in Hz. */
		_Check_return_ inline float GetStoredBestHz(void) const
		{
			return 1.0f / GetStoredBest();
		}

		/* Gets the exclusive worst draw rate in Hz. */
		_Check_return_ inline float GetExclusiveWorstHz(void) const
		{
			return 1.0f / GetExclusiveWorst();
		}

		/* Gets the exclusive best draw rate in Hz. */
		_Check_return_ inline float GetExclusiveBestHz(void) const
		{
			return 1.0f / GetExclusiveBest();
		}

	protected:
		/* Returns false. */
		_Check_return_ virtual bool TryAppendBufferOnUpdate(_In_ float dt, _Out_ float *value) const
		{
			return false;
		}

		/* Sets value to delta time. */
		_Check_return_ virtual bool TryAppendBUfferOnDraw(_In_ float dt, _Out_ float *value) const
		{
			*value = dt;
			return true;
		}

		/* Updates the display values. */
		virtual void CalculateDisplayValues(_Out_ float *avrg, _Out_ float *worst, _Out_ float *best) const
		{
			/* Reset values and create temporate sum. */
			float sum = 0.0f;
			*worst = minv<float>();
			*best = maxv<float>();

			/* Update sum, worst and best. */
			for (size_t i = 0; i < buffer.size(); i++)
			{
				float cur = buffer.at(i);

				sum += cur;
				if (*worst < cur) *worst = cur;
				if (*best > cur) *best = cur;
			}

			/* Set average. */
			*avrg = sum / buffer.size();
		}

		/* Updates the exclusive display values. */
		virtual void CalculateExclusives(_Inout_ float *exclusiveWorst, _Inout_ float *exclusiveBest, _In_ const float *lastWorst, _In_ const float *lastBest) const
		{
			if (*exclusiveWorst < *lastWorst) *exclusiveWorst = *lastWorst;
			if (*exclusiveBest > *lastBest) *exclusiveBest = *lastBest;
		}
	};
}