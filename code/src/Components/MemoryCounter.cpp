#include "Components\MemoryCounter.h"
#include "Graphics\Diagnostics\DeviceInfo.h"

Plutonium::MemoryCounter::MemoryCounter(Game * game, size_t bufferSize, int32 rate)
	: GameComponent(game), maxSize(bufferSize),
	avrg(0), worst(0), best(0),
	vavrg(0), vworst(0), vbest(0),
	gpuavrg(0), gpuworst(0), gpubest(0),
	updElap(0.0f), updIntrv(1.0f / rate)
{}

void Plutonium::MemoryCounter::Update(float dt)
{
	/* Add current frame to buffer. */
	cpuBuffer.push_back(_CrtGetMemStats());
	gpuBuffer.push_back(_CrtGetUsedGPUMemory());

	/* Make sure we don't store more than is requested. */
	while (cpuBuffer.size() > maxSize) cpuBuffer.pop_front();
	while (gpuBuffer.size() > maxSize) gpuBuffer.pop_front();

	/* Make sure we don't update too much. */
	if ((updElap += dt) > updIntrv)
	{
		updElap = 0.0f;

		/* Reset general values. */
		avrg = 0; vavrg = 0; gpuavrg = 0;
		worst = 0; vworst = 0; gpuworst = 0;
		best = maxv<uint64>(); vbest = maxv<uint64>(); gpubest = maxv<uint64>();

		/* Update general stats. */
		for (size_t i = 0; i < cpuBuffer.size(); i++)
		{
			/* Get value within buffer. */
			MemoryFrame frame = cpuBuffer.at(i);
			uint64 mem = gpuBuffer.at(i);

			/* Update averages. */
			avrg += frame.UsedRam; 
			vavrg += frame.UsedVRam;
			gpuavrg += mem;

			/* Update worst. */
			if (worst < frame.UsedRam) worst = frame.UsedRam;
			if (vworst < frame.UsedVRam) vworst = frame.UsedVRam;
			if (gpuworst < mem) gpuworst = mem;

			/* Update best. */
			if (best > frame.UsedRam) best = frame.UsedRam;
			if (vbest > frame.UsedVRam) vbest = frame.UsedVRam;
			if (gpubest > mem) gpubest = mem;
		}

		avrg /= cpuBuffer.size();
		vavrg /= cpuBuffer.size();
		gpuavrg /= gpuBuffer.size();
	}
}