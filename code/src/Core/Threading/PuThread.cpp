#include "Core/Threading/PuThread.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Math/Basics.h"
#include "Config.h"

using namespace Pu;

static vector<PuThread*> activeThreads;
static std::mutex bufferLock;

/*
!!!!!!!!!! Remember !!!!!!!!!!
If this function is removed or the name is changed the stack trace logger needs to be updated!
!!!!!!!!!! Remember !!!!!!!!!!
*/
void Pu::_CrtPuThreadStart(uint32 id, const wstring & name)
{
	/* Start by setting the name of the thread on debug mode, on release cast to void to remove warning. */
#ifdef _DEBUG
	_CrtSetCurrentThreadName(name);
#else 
	(void)name;
#endif

	/*
	Lock the thread buffer to make sure we don't mess up between threads.
	Using a lock because this operation should always be fast and doesn't need atmoic optimization.
	*/
	bufferLock.lock();

	for (PuThread *cur : activeThreads)
	{
		if (cur->id == id)
		{
			/* Unlock the buffer because we found our thread. */
			bufferLock.unlock();

			/* Launch the thread object's main. */
			cur->_CrtPuThreadMain();
			return;
		}
	}

	/* Couldn't find thread in buffer, this should never occur! */
	bufferLock.unlock();
	Log::Error("Unable to find thread object for thread %zu (%lu)!", id, _CrtGetCurrentThreadId());
}

Pu::PuThread::PuThread(const wstring & name)
	: id(static_cast<uint32>(random(0, 1024))), stopped(false)
{
	/* Add thread object to buffer. */
	bufferLock.lock();
	activeThreads.push_back(this);
	bufferLock.unlock();

	/* Start underlying thread. */
	thread = new std::thread(_CrtPuThreadStart, id, name);
}

Pu::PuThread::~PuThread(void) noexcept
{
	/* Wait for the thread to end and delete thread object. */
	Wait();
	delete thread;

	/* Safely remove the thread from the buffer. */
	bufferLock.lock();
	activeThreads.remove(this);
	bufferLock.unlock();
}

bool Pu::PuThread::Wait(void) const
{
	if (thread->joinable())
	{
		thread->join();
		stopped = true;
		return true;
	}
	else if (!stopped)
	{
		thread->detach();
		return false;
	}

	return true;
}

void Pu::PuThread::Sleep(uint64 milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

size_t Pu::PuThread::GetMaxConcurrent(void)
{
	return std::thread::hardware_concurrency();
}

bool Pu::PuThread::WaitAll(const vector<PuThread*> & threads)
{
	bool noDetaches = true;

	for (const PuThread * thread : threads)
	{
		noDetaches = noDetaches && thread->Wait();
	}

	return noDetaches;
}