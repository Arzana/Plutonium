#pragma once
#include "Core/String.h"
#include <thread>
#include <atomic>

namespace Pu
{
	/* Defines a named thread object. */
	class PuThread
	{
	public:
		/* Initializes a new instance of a named thread. */
		PuThread(_In_ const wstring &name);
		PuThread(_In_ const PuThread &value) = delete;
		PuThread(_In_ PuThread &&value) = delete;
		/* Waits for the thread to stop and releases it's resources. */
		~PuThread(void) noexcept;

		_Check_return_ PuThread& operator =(_In_ const PuThread &other) = delete;
		_Check_return_ PuThread& operator =(_In_ PuThread &&other) = delete;

		/*
		Waits for the thread to stop excecution.
		Returns true if the thread was safely stopped.
		Returns false if the thread had to be detached to stop.
		*/
		_Check_return_ virtual bool Wait(void) const;
		/* Locks this thread to a specific CPU core. */
		void Lock(_In_ uint64 core);

		/* Locks the calling thread to a specific CPU core. */
		static void LockCalling(_In_ uint64 core);
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
		friend void _CrtPuThreadStart(PuThread *thread, const wstring&);

		std::thread *thread;
		mutable bool stopped;
	};
}