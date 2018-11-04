#include "Core\Threading\TickThread.h"
#include "Core\Threading\ThreadUtils.h"
#include <chrono>

Plutonium::TickThread::TickThread(const char * name, uint32 cooldown)
	: PuThread(name), cooldown(cooldown), 
	Initialize("ThreadInit"), Tick("ThreadTick"), Terminate("ThreadTerm")
{
	allow.store(true);
}

Plutonium::TickThread::~TickThread(void) noexcept
{
	StopWait();
}

void Plutonium::TickThread::Stop(void) const
{
	allow.store(false);
}

bool Plutonium::TickThread::StopWait(void)
{
	Stop();
	return Wait();
}

void Plutonium::TickThread::Run(void)
{
	const char *name = _CrtGetCurrentThreadName();

	/* Initialize running thread. */
	LOG("Initializing thread: %s.", name);
	Initialize.Post(this, EventArgs());

	/* Run until Stop is called. */
	while (allow.load())
	{
		Tick.Post(this, EventArgs());

		/* Make sure we don't update too often. */
		if (cooldown) std::this_thread::sleep_for(std::chrono::milliseconds(cooldown));
	}

	/* Finalize running thread. */
	LOG("Terminating thread: %s.", name);
	Terminate.Post(this, EventArgs());

	free_s(name);
}