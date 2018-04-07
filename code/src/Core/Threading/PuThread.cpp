#include "Core\Threading\PuThread.h"
#include "Core\Threading\ThreadUtils.h"
#include "Core\SafeMemory.h"
#include <chrono>
#include <vector>
#include <mutex>

/* Global thread storage. */
Plutonium::int32 idCnt = 0;
std::vector<Plutonium::PuThread*> activeThreads;
std::mutex bufferLock;

/* 
!!!!!!!!!! Remember !!!!!!!!!!
If this function is removed or the name is changed the stack trace logger needs to be updated!
!!!!!!!!!! Remenber!!!!!!!!!!
*/
void Plutonium::_CrtPuThreadStart(uint32 id, const char *name)
{
	/* Start by setting the name of the thread on debug mode. */
#if defined(DEBUG)
	_CrtSetCurrentThreadName(name);
#endif

	/*
	Lock the thread buffer to make sure we don't mess up between threads. 
	Using a lock because this operation should always be fast and doesn't need atmoic optimization.
	*/
	bufferLock.lock();

	for (size_t i = 0; i < activeThreads.size(); i++)
	{
		/* Find requested thread. */
		PuThread *cur = activeThreads.at(i);
		if (cur->id == id)
		{
			/* Unlock the buffer because we found our thread. */
			bufferLock.unlock();

			/* Wait for the thread to be started. */
			while (!cur->started.load()) PuThread::Sleep(100);

			/* Launch the thread object's main. */
			cur->_CrtPuThreadMain();

			/* Make sure the object's stop indintifier is set, might cause problems with detach. */
			cur->stopped.store(true);
			return;
		}
	}

	/* Couldn't find thread in buffer, this should never occur! */
	bufferLock.unlock();
	LOG_THROW("Unable to find thread object for thread %zu (%lu)!", id, _CrtGetCurrentThreadId());
}

Plutonium::PuThread::PuThread(const char * name)
	: id(idCnt++)
{
	/* Set thread state indentifiers. */
	started.store(false);
	stopped.store(false);

	/* Add thread object to buffer. */
	bufferLock.lock();
	activeThreads.push_back(this);
	bufferLock.unlock();

	/* Start underlying thread. */
	thread = new std::thread(_CrtPuThreadStart, id, name);
}

Plutonium::PuThread::~PuThread(void) noexcept
{
	/* Safely remove the thread from the buffer. */
	bufferLock.lock();
	for (size_t i = 0; i < activeThreads.size(); i++)
	{
		if (activeThreads.at(i) == this)
		{
			activeThreads.erase(activeThreads.begin() + i);
			break;
		}
	}
	bufferLock.unlock();

	/* Wait for the thread to end and delete thread object. */
	Wait();
	delete_s(thread);
}

void Plutonium::PuThread::Start(void)
{
	/* On debug mode, check if the function is called as it should (only once). */
#if defined(DEBUG)
	if (started.load())
	{
		LOG_WAR_IF(!stopped.load(), "Attempting to start already started thread!");
		ASSERT_IF(!stopped.load(), "Attempting to start stopped thread!");
		return;
	}
#endif

	started.store(true);
}

void Plutonium::PuThread::Sleep(uint64 milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

bool Plutonium::PuThread::Wait(void) const
{
	/* If available use the threads build in wait system. */
	if (thread->joinable()) thread->join();
	else
	{
		/* Define the sleep time as 0.1 seconds and the detach threshold as 10 seconds. */
		constexpr uint64 SLEEP_TIME = 100;
		constexpr uint64 SLEEP_MAX = 10000;

		/* Wait untill stopped is set. */
		uint64 elapsed = 0;
		while (!stopped.load())
		{
			/* Sleep to make sure we don't overload the CPU. */
			PuThread::Sleep(SLEEP_TIME);

			/* Check if threshold is reached. */
			if ((elapsed += SLEEP_TIME) > SLEEP_MAX)
			{
				/* 
				This should not occur but it's build in to make sure threads always safely stop.
				So if it does occur make sure we log it to the user so they can fix the issue.
				*/
				LOG_WAR("Thread %zu took longer then %lu milliseconds to stop after wait command, detaching thread!", id, SLEEP_MAX);
				thread->detach();
				return false;
			}
		}
	}

	return true;
}