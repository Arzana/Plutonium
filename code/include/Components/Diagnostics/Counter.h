#pragma once
#include "Components\GameComponent.h"
#include <deque>

namespace Plutonium
{
	/* Provides a simple base object for counters. */
	template <typename _BufferedType, typename _DisplayType>
	struct Counter
		: GameComponent
	{
	public:
		/* Defines the type of this counter. */
		using CounterType = Counter<_BufferedType, _DisplayType>;

		/* Initializes a new instance of a counter with a specified average buffer size and update rate (in Hz). */
		Counter(_In_ Game *game, _In_opt_ size_t bufferSize = 100, _In_opt_ float rate = 1.0f)
			: GameComponent(game), maxSize(bufferSize),
			interv(1.0f / rate), elap(0.0f),
			avrg(), worst(), best(), exlWorst(), exlBest()
		{
			buffer.resize(bufferSize);
		}

		Counter(_In_ const CounterType &value) = delete;
		Counter(_In_ CounterType &&value) = delete;

		_Check_return_ CounterType& operator =(_In_ const CounterType &other) = delete;
		_Check_return_ CounterType& operator =(_In_ CounterType &&other) = delete;

		/* Gets the last stored value, returns the default value if the buffer is empty. */
		_Check_return_ inline _DisplayType GetCurrent(void) const
		{
			return buffer.size() > 0 ? buffer.at(buffer.size() - 1) : _DisplayType();
		}

		/* Gets the average of the stored values, returns the default if the average has not yet been calculated. */
		_Check_return_ inline _DisplayType GetAverage(void) const
		{
			return avrg;
		}

		/* Gets the worst of the stored values, returns the default if the worst has not yet been calculated. */
		_Check_return_ inline _DisplayType GetStoredWorst(void) const
		{
			return worst;
		}

		/* Gets the best of the stored values, returns the default if the best has not yet been calculated. */
		_Check_return_ inline _DisplayType GetStoredBest(void) const
		{
			return best;
		}

		/* Gets the worst value over the entire time that this counter has been alive, returns the default if the exclusive worst has not yet been calculated. */
		_Check_return_ inline _DisplayType GetExclusiveWorst(void) const
		{
			return exlWorst;
		}

		/* Gets the best value over the entire time that this counter has been alive, returns the default if the exclusive best has not yet been calculated. */
		_Check_return_ inline _DisplayType GetExclusiveBest(void) const
		{
			return exlBest;
		}

	protected:
		/* The storage buffer. */
		std::deque<_BufferedType> buffer;

		/* Defines the method used for appending to the buffer on Update, returns whether the value should be added. */
		_Check_return_ virtual bool TryAppendBufferOnUpdate(_In_ float dt, _Out_ _BufferedType *value) const = 0;
		/* Defines the method used for appending to the buffer on Render, returns whether the value should be added. */
		_Check_return_ virtual bool TryAppendBUfferOnDraw(_In_ float dt, _Out_ _BufferedType *value) const = 0;
		/* Defines the method used for calculating the display values. */
		virtual void CalculateDisplayValues(_Out_ _DisplayType *avrg, _Out_ _DisplayType *worst, _Out_ _DisplayType *best) const = 0;
		/* Defines the method used for checking for new exclusive best and worst values. */
		virtual void CalculateExclusives(_Inout_ _DisplayType *exclusiveWorst, _Inout_ _DisplayType *exclusiveBest, _In_ const _DisplayType *lastWorst, _In_ const _DisplayType *lastBest) const = 0;

		/* Attenpts to update the counter. */
		virtual void Update(_In_ float dt) override
		{
			/* Try to push a new value to the buffer. */
			_BufferedType value;
			if (TryAppendBufferOnUpdate(dt, &value)) AppendToBuffer(value);

			/* Update the display values once the interval is reached. */
			if ((elap += dt) >= interv)
			{
				elap = 0.0f;
				CalculateDisplayValues(&avrg, &worst, &best);
				CalculateExclusives(&exlWorst, &exlBest, &worst, &best);
			}
		}

		/* Attempts to update the counter on render. */
		virtual void Render(_In_ float dt) override
		{
			/* Try to push a new value to the buffer. */
			_BufferedType value;
			if (TryAppendBUfferOnDraw(dt, &value)) AppendToBuffer(value);
		}

	private:
		const size_t maxSize;
		_DisplayType avrg, worst, best, exlWorst, exlBest;
		float interv, elap;

		void AppendToBuffer(_BufferedType &value)
		{
			/* Append value to the buffer and make sure we don't store more tan is requested. */
			buffer.push_back(value);
			while (buffer.size() > maxSize) buffer.pop_front();
		}
	};
}