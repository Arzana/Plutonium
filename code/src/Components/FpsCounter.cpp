#include "Components\FpsCounter.h"

Plutonium::FpsCounter::FpsCounter(Game * game, size_t bufferSize, int32 rate)
	: GameComponent(game), maxSize(bufferSize),
	avrg(maxv<float>()), worst(maxv<float>()), best(maxv<float>()),
	updElap(0.0f), updIntrv(1.0f / rate)
{
	buffer.resize(bufferSize);
}

void Plutonium::FpsCounter::Render(float dt)
{
	/* Add current fps to buffer. */
	buffer.push_back(dt);

	/* Make sure we don't store more than is requested. */
	while (buffer.size() > maxSize) buffer.pop_front();

	/* Make sure we don't update too much. */
	if ((updElap += dt) >= updIntrv)
	{
		updElap = 0.0f;

		/* Reset general stats. */
		avrg = 0.0f;
		worst = minv<float>();
		best = maxv<float>();

		/* Update general stats. */
		for (size_t i = 0; i < buffer.size(); i++)
		{
			float cur = buffer.at(i);

			avrg += cur;
			if (worst < cur) worst = cur;
			if (best > cur) best = cur;
		}

		avrg /= buffer.size();
	}
}
