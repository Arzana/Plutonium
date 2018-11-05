#pragma once
#include "Core\Events\EventBus.h"
#include "Core\Events\EventArgs.h"
#include "PuThread.h"

namespace Pu
{
	/* Defines a continually ticking thread. */
	struct TickThread
		: PuThread
	{
	public:
		/* Occurs before the tick routine is started. */
		EventBus<const TickThread, EventArgs> Initialize;
		/* Called untill a stop is requested. */
		EventBus<TickThread, EventArgs> Tick;
		/* Occurs after the tick routine has stopped. */
		EventBus<const TickThread, EventArgs> Terminate;

		/* Initializes and starts a new instance of a continually ticking thread (cooldown is in milliseconds). */
		TickThread(_In_ const char *name, _In_opt_ uint32 cooldown = 0);
		TickThread(_In_ const TickThread &value) = delete;
		TickThread(_In_ TickThread &&value) = delete;
		/* Stops the thread (if needed) and releases the resources allocated. */
		~TickThread(void) noexcept;

		_Check_return_ TickThread& operator =(_In_ const TickThread &other) = delete;
		_Check_return_ TickThread& operator =(_In_ TickThread &&other) = delete;

		/* Requests the thread to stop excecution. */
		void Stop(void);
		/* Requests the thread to stop excecution and waits for the thread to terminate. */
	 	_Check_return_ bool StopWait(void);

	protected:
		/* Entry point for the thread. */
		virtual void _CrtPuThreadMain(void) final override;

	private:
		mutable std::atomic_bool allow;
		const uint32 cooldown;
	};
}