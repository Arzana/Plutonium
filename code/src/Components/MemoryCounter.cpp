#include "Components\MemoryCounter.h"

Plutonium::MemoryCounter::MemoryCounter(Game * game, size_t bufferSize, int32 rate)
	: GameComponent(game), maxSize(bufferSize),
	avrg(0), worst(0), best(0),
	vavrg(0), vworst(0), vbest(0),
	updElap(0.0f), updIntrv(1.0f / rate)
{}

void Plutonium::MemoryCounter::Update(float dt)
{
	/* Add current frame to buffer. */
	MemoryFrame frame = _CrtGetMemStats();
	buffer.push_back(frame);

	/* Make sure we don't store more than is requested. */
	while (buffer.size() > maxSize) buffer.pop_front();

	/* Make sure we don't update too much. */
	if ((updElap += dt) > updIntrv)
	{
		updElap = 0.0f;

		/* Reset general values. */
		avrg = 0; vavrg = 0;
		worst = 0; vworst = 0;
		best = UINT64_MAX; vbest = UINT64_MAX;

		/* Update general stats. */
		for (size_t i = 0; i < buffer.size(); i++)
		{
			MemoryFrame cur = buffer.at(i);

			avrg += cur.UsedRam; vavrg += cur.UsedVRam;
			if (worst < cur.UsedRam) worst = cur.UsedRam;
			if (vworst < cur.UsedVRam) vworst = cur.UsedVRam;
			if (best > cur.UsedRam) best = cur.UsedRam;
			if (vbest > cur.UsedVRam) vbest = cur.UsedVRam;
		}

		avrg /= buffer.size();
		vavrg /= buffer.size();
	}
}