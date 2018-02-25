#include "Components\FpsCounter.h"

Plutonium::FpsCounter::FpsCounter(Game * game, size_t bufferSize)
	: GameComponent(game), maxSize(bufferSize), 
	avrg(0.0f), worst(0.0f), best(0.0f)
{
	buffer.resize(bufferSize);
}

void Plutonium::FpsCounter::Render(float dt)
{
	/* Add current fps to buffer. */
	buffer.push_back(dt);

	/* Make sure we don't store more than is requested. */
	while (buffer.size() > maxSize) buffer.pop_front();

	/* Reset general stats. */
	avrg = 0.0f;
	worst = FLT_MIN;
	best = FLT_MAX;

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
