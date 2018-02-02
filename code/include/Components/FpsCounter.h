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
		FpsCounter(_In_ Game *game, _In_ size_t bufferSize = 100);

		/* Gets the update rate in Hz. */
		_Check_return_ float GetUps(void) const
		{
			return ups;
		}

		/* Gets the current draw rate in Hz. */
		_Check_return_ float GetCurFps(void) const
		{
			ASSERT_IF(buffer.size() < 1, "Buffer empty!", "Cannot current fps when no frame has been drawn yet!");
			return buffer.at(buffer.size() - 1);
		}

		/* Gets the average draw rate in Hz over the amount of frames specified. */
		_Check_return_ float GetAvrgFps(void) const
		{
			return avrg;
		}

	protected:
		/* Updates the ups of the fps counter. */
		virtual void Update(_In_ float dt) override;
		/* Updates the fps of the fps counter. */
		virtual void Render(_In_ float dt) override;

	private:
		size_t maxSize;
		std::deque<float> buffer;
		float avrg, ups;
	};
}