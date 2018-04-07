#pragma once
#include "Core\Events\EventBus.h"
#include "Core\Events\EventArgs.h"
#include "PuThread.h"

namespace Plutonium
{
	/* Defines a continually ticking thread. */
	typedef const struct TickThread
		: PuThread
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

		/* Requests the thread to stop excecution. */
		void Stop(void) const;
		/* Requests the thread to stop excecution and waits for the thread to terminate. */
		bool StopWait(void);

	protected:
		/* Entry point for the thread. */
		inline virtual void _CrtPuThreadMain(void) override
		{
			Run();
		}

	private:
		mutable std::atomic_bool allow;
		const uint32 cooldown;

		void Run(void);
	} *TickThreadHandler;
}