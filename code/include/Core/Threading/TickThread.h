#pragma once
#include "Core\Events\EventBus.h"
#include "Core\Events\UserEventArgs.h"
#include "PuThread.h"

namespace Pu
{
	/* Defines a continually ticking thread. */
	class TickThread
		: public PuThread
	{
	public:
		/* Occurs before the tick routine is started. */
		EventBus<const TickThread, UserEventArgs> Initialize;
		/* Called untill a stop is requested. */
		EventBus<TickThread, UserEventArgs> Tick;
		/* Occurs after the tick routine has stopped. */
		EventBus<const TickThread, UserEventArgs> Terminate;

		/* Initializes and starts a new instance of a continually ticking thread (cooldown is in milliseconds). */
		TickThread(_In_ const wstring &name, _In_opt_ uint32 cooldown = 0, _In_opt_ const void *param = nullptr);
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
		void _CrtPuThreadMain(void) final;

	private:
		mutable std::atomic_bool allow;
		const uint32 cooldown;
		const UserEventArgs args;
	};
}