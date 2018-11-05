#include "Core/Threading/TickThread.h"
#include "Core/Threading/ThreadUtils.h"

Pu::TickThread::TickThread(const char * name, uint32 cooldown)
	: PuThread(name), cooldown(cooldown),
	Initialize("TickThreadInitialize"), Tick("TickThreadTick"), Terminate("TickThreadTerminate")
{
	allow.store(true);
}

Pu::TickThread::~TickThread(void) noexcept
{
}

void Pu::TickThread::Stop(void)
{
	StopWait();
}

bool Pu::TickThread::StopWait(void)
{
	Stop();
	return Wait();
}

void Pu::TickThread::_CrtPuThreadMain(void)
{
	const string name = _CrtGetCurrentThreadName();
	TickThread &self = *this;

	/* Initialize running thread. */
	Log::Verbose("Initializing thread: %s.", name.c_str());
	Initialize.Post(self, EventArgs());

	/* Run until Stop is called. */
	while (allow.load())
	{
		Tick.Post(self, EventArgs());

		/* Make sure we don't update too often. */
		if (cooldown) std::this_thread::sleep_for(std::chrono::milliseconds(cooldown));
	}

	/* Finalize running thread. */
	Log::Verbose("Terminating thread: %s.", name.c_str());
	Terminate.Post(self, EventArgs());
}