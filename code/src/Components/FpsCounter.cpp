#include "Components\FpsCounter.h"

Plutonium::FpsCounter::FpsCounter(Game * game, size_t bufferSize)
	: GameComponent(game), maxSize(bufferSize), avrg(0.0f), ups(0.0f)
{
	buffer.resize(bufferSize);
}

void Plutonium::FpsCounter::Update(float dt)
{
	ups = 1.0f / dt;
}

void Plutonium::FpsCounter::Render(float dt)
{
	/* Add current fps to buffer. */
	float fps = 1.0f / dt;
	buffer.push_back(fps);

	/* Make sure we don't store more than is requested. */
	while (buffer.size() > maxSize) buffer.pop_front();

	/* Update fps average. */
	avrg = 0.0f;
	for (size_t i = 0; i < buffer.size(); i++) avrg += buffer.at(i);
	avrg /= buffer.size();
}
