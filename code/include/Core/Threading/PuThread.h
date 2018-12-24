#pragma once
#include "Core/Math/Constants.h"
#include "Core/Collections/vector.h"
#include <thread>
#include <atomic>

namespace Pu
{
	/* Defines a named thread object. */
	class PuThread
	{
	public:
		/* Initializes a new instance of a named thread. */
		PuThread(_In_ const char *name);
		PuThread(_In_ const PuThread &value) = delete;
		PuThread(_In_ PuThread &&value) = delete;
		/* Waits for the thread to stop and releases it's resources. */
		~PuThread(void) noexcept;

		_Check_return_ PuThread& operator =(_In_ const PuThread &other) = delete;
		_Check_return_ PuThread& operator =(_In_ PuThread &&other) = delete;

		/* Starts the excecution of this thread. */
		virtual void Start(void);
		/*
		Waits for the thread to stop excecution.
		Returns true if the thread was safely stopped.
		Returns false if the thread had to be detached to stop.
		*/
		_Check_return_ virtual bool Wait(void) const;

		/* Commands the caller thread to sleep for a specified amount of time. */
		static void Sleep(_In_ uint64 milliseconds);
		/* Gets the maximum amount of concurrent threads supported. */
		static size_t GetMaxConcurrent(void);
		/*
		Waits for the specified threads to stop excecution. 
		Retuns true if all threads were safelty stopped.
		Returns false if a thread had to be deteched to stop.
		*/
		_Check_return_ static bool WaitAll(_In_ const vector<PuThread*> &threads);

	protected:
		/* Entry point for the thread, gets called after Start on the underlying thread. */
		virtual void _CrtPuThreadMain(void) = 0;

	private:
		friend void _CrtPuThreadStart(uint32, const char*);

		std::thread *thread;
		std::atomic_bool started, stopped;
		const uint32 id;
	};
}