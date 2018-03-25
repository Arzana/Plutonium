#pragma once
#include "GameComponent.h"
#include <deque>

namespace Plutonium
{
	/* Provides a simple fps counter with a specified buffer size. */
	struct FpsCounter
		: GameComponent
	{
	public:
		/* Initializes a new instance of an fps counter. */
		FpsCounter(_In_ Game *game, _In_opt_ size_t bufferSize = 100, _In_opt_ int32 rate = 60);
		FpsCounter(_In_ const FpsCounter &value) = delete;
		FpsCounter(_In_ FpsCounter &&value) = delete;

		_Check_return_ FpsCounter& operator =(_In_ const FpsCounter &other) = delete;
		_Check_return_ FpsCounter& operator =(_In_ FpsCounter &&other) = delete;

		/* Gets the current draw rate in milliseconds. */
		_Check_return_ float GetCurMs(void) const
		{
			ASSERT_IF(buffer.size() < 1, "Buffer empty!", "Cannot get current fps when no frame has been drawn yet!");
			return buffer.at(buffer.size() - 1);
		}

		/* Gets the average draw rate in milliseconds over the amount of frames specified. */
		_Check_return_ float GetAvrgMs(void) const
		{
			return avrg;
		}

		/* Gets the worst draw rate in milliseconds over the amount of frames specified. */
		_Check_return_ float GetWorstMs(void) const
		{
			return worst;
		}

		/* Gets the best draw rate in milliseconds over the amount of frames specified. */
		_Check_return_ float GeBestMs(void) const
		{
			return best;
		}

		/* Gets the current draw rate in milliseconds. */
		_Check_return_ float GetCurHz(void) const
		{
			return 1.0f / GetCurMs();
		}

		/* Gets the average draw rate in Hz over the amount of frames specified. */
		_Check_return_ float GetAvrgHz(void) const
		{
			return 1.0f / GetAvrgMs();
		}

		/* Gets the worst draw rate in Hz over the amount of frames specified. */
		_Check_return_ float GetWorstHz(void) const
		{
			return 1.0f / GetWorstMs();
		}

		/* Gets the best draw rate in Hz over the amount of frames specified. */
		_Check_return_ float GeBestHz(void) const
		{
			return 1.0f / GeBestMs();
		}

	protected:
		/* Updates the fps of the fps counter. */
		virtual void Render(_In_ float dt) override;

	private:
		size_t maxSize;
		std::deque<float> buffer;
		float avrg, worst, best;
		float updIntrv, updElap;
	};
}