#include "Graphics/Vulkan/Queue.h"

using namespace Pu;

Pu::Queue::Queue(Queue && value)
	: hndl(value.hndl)
{
	value.hndl = nullptr;
}

Queue & Pu::Queue::operator=(Queue && other)
{
	if (this != &other)
	{
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::Queue::Queue(QueueHndl hndl)
	: hndl(hndl)
{}