#include "Core/Threading/TickThread.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/Logging.h"

Pu::TickThread::TickThread(const char * name, uint32 cooldown, const void * param)
	: PuThread(name), cooldown(cooldown), args(param),
	Initialize("TickThreadInitialize"), Tick("TickThreadTick"), Terminate("TickThreadTerminate")
{
	allow.store(true);
}

Pu::TickThread::~TickThread(void) noexcept
{
	Stop();
}

void Pu::TickThread::Stop(void)
{
	allow.store(false);
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
	Initialize.Post(self, args);

	/* Run until Stop is called. */
	while (allow.load())
	{
		Tick.Post(self, args);

		/* Make sure we don't update too often. */
		if (cooldown) std::this_thread::sleep_for(std::chrono::milliseconds(cooldown));
	}

	/* Finalize running thread. */
	Log::Verbose("Terminating thread: %s.", name.c_str());
	Terminate.Post(self, args);
}