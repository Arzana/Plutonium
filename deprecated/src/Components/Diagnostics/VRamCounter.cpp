#include "Components\Diagnostics\VRAMCounter.h"
#include "Game.h"

Plutonium::VRamCounter::VRamCounter(Game * game, size_t bufferSize, float rate)
	: Counter(game, bufferSize, rate)
{
	budget = game->GetGraphics()->GetInfo()->FrameBufferSize;
}