#pragma once
#include "Core\Events\EventBus.h"
#include "Core\Events\EventArgs.h"
#include <thread>
#include <atomic>

namespace Plutonium
{
	/* Defines a continually ticking thread. */
	typedef const struct TickThread
	{
	public:
		/* Occurs before the tick routine is started. */
		EventBus<TickThread, EventArgs> Initialize;
		/* Called untill a stop is requested. */
		EventBus<TickThread, EventArgs> Tick;
		/* Occurs after the tick routine has stopped. */
		EventBus<TickThread, EventArgs> Terminate;

		/* Initializes and starts a new instance of a continually ticking thread (cooldown is in milliseconds). */
		TickThread(_In_ const char *name, _In_opt_ uint32 cooldown = 0);
		TickThread(_In_ const TickThread &value) = delete;
		TickThread(_In_ TickThread &&value) = delete;
		/* Stops the thread (if needed) and releases the resources allocated. */
		~TickThread(void) noexcept;

		_Check_return_ TickThread& operator =(_In_ const TickThread &other) = delete;
		_Check_return_ TickThread& operator =(_In_ TickThread &&other) = delete;

		/* Starts the thread. */
		void Start(void);
		/* Requests the thread to stop excecution. */
		void Stop(void) const;
		/* Requests the thread to stop excecution and waits for the thread to terminate. */
		void StopWait(void);

	private:
		friend void ThreadMain(uint32);

		std::thread *thread;
		std::atomic_bool running, started;
		mutable std::atomic_bool allow;
		const uint32 id, cooldown;
#if defined(DEBUG)
		const char *name;
#endif

		void Run(void);
	} *TickThreadHandler;
}