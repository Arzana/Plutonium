#include "Core/Threading/PuThread.h"
#include "Core/Platform/Windows/Windows.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Math/Basics.h"
#include "Config.h"

using namespace Pu;

/*
!!!!!!!!!! Remember !!!!!!!!!!
If this function is removed or the name is changed the stack trace logger needs to be updated!
!!!!!!!!!! Remember !!!!!!!!!!
*/
void Pu::_CrtPuThreadStart(PuThread *thread, const wstring & name)
{
	/* Start by setting the name of the thread on debug mode, on release cast to void to remove warning. */
#ifdef _DEBUG
	_CrtSetCurrentThreadName(name);
#else 
	(void)name;
#endif

	/* Sleep for one millisecond to allow the constructor to finish execution. */
	PuThread::Sleep(1ull);
	thread->_CrtPuThreadMain();
}

Pu::PuThread::PuThread(const wstring & name)
	: stopped(false)
{
	/* Start underlying thread. */
	thread = new std::thread(_CrtPuThreadStart, this, name);
}

Pu::PuThread::~PuThread(void) noexcept
{
	/* Wait for the thread to end and delete thread object. */
	Wait();
	delete thread;
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

void Pu::PuThread::Lock(uint64 core)
{
#ifdef _WIN32
	if (!SetThreadAffinityMask(thread->native_handle(), (1ull << core)))
	{
		Log::Error("Failed to lock thread to CPU core %zu (%ls)!", core, _CrtGetErrorString().c_str());
	}
#else
	Log::Warning("Cannot lock thread to specific CPU core on this platform!");
#endif
}

void Pu::PuThread::LockCalling(uint64 core)
{
#ifdef _WIN32
	if (!SetThreadAffinityMask(GetCurrentThread(), (1ull << core)))
	{
		Log::Error("Failed to lock thread to CPU core %zu (%ls)!", core, _CrtGetErrorString().c_str());
	}
#else
	Log::Warning("Cannot lock thread to specific CPU core on this platform!");
#endif
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