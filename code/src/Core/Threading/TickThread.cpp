#include "Core\Threading\TickThread.h"
#include "Core\SafeMemory.h"
#include "Core\Threading\ThreadUtils.h"
#include <chrono>
#include <vector>
#include <mutex>

#define thread_sleep(amnt)		std::this_thread::sleep_for(std::chrono::milliseconds(amnt))

Plutonium::uint32 idCnt = 0;
std::vector<Plutonium::TickThread*> activeThreads;
std::mutex threadBufferLock;

void Plutonium::ThreadMain(uint32 id)
{
	/* Lock the thread buffer to make sure we don't mess up between threads. */
	threadBufferLock.lock();

	for (size_t i = 0; i < activeThreads.size(); i++)
	{
		TickThread *cur = activeThreads.at(i);
		if (cur->id == id)
		{
			/* Make sure we unlock the buffer. */
			threadBufferLock.unlock();

			/* Wait untill thread can be started. */
			while (!cur->started.load()) thread_sleep(100);

			/* Launch run loop within thread object. */
			cur->Run();
			return;
		}
	}

	/* Couldn't find thread in buffer, this should never occur. */
	threadBufferLock.unlock();
	LOG_THROW("Unable to find thread object for thread %zu (%lu)!", id, _threadid);
}

Plutonium::TickThread::TickThread(const char * name, uint32 cooldown)
	: id(idCnt++), cooldown(cooldown),
	Initialize("ThreadInit"), Tick("ThreadTick"), Terminate("ThreadTerm")
#if defined(DEBUG)
	, name(name)
#endif
{
	started.store(false);
	running.store(false);
	allow.store(true);

	/* Push thread to buffer. */
	threadBufferLock.lock();
	activeThreads.push_back(this);
	threadBufferLock.unlock();

	/* Starts the thread. */
	thread = new std::thread(ThreadMain, id);
}

Plutonium::TickThread::~TickThread(void) noexcept
{
	/* Safely remove the thread from the buffer. */
	threadBufferLock.lock();
	for (size_t i = 0; i < activeThreads.size(); i++)
	{
		if (activeThreads.at(i) == this)
		{
			activeThreads.erase(activeThreads.begin() + i);
			break;
		}
	}
	threadBufferLock.unlock();

	/* Make sure we release the thread. */
	if (running.load()) Stop();
	if (thread->joinable()) thread->join();
	delete_s(thread);
}

void Plutonium::TickThread::Start(void)
{
	/* Start thread activator. */
	ASSERT_IF(!allow.load(), "Attempting to start stopped thread!");
	started.store(true);
}

void Plutonium::TickThread::Stop(void) const
{
	/* Register stop command for underlying thread. */
	ASSERT_IF(!started.load(), "Attempting to stop not-started thread!");
	if (!running.load()) LOG_WAR("Unable to stop thread that is no longer active!");
	else allow.store(false);
}

void Plutonium::TickThread::StopWait(void)
{
	/* Stops the thread and waits for the thread to end excecution, waits 100 miliseconds between checks. */
	Stop();
	while (running.load()) thread_sleep(100);
}

void Plutonium::TickThread::Run(void)
{
	/* On debug mode set the thread name as its description. */
#if defined(DEBUG)
	_CrtSetCurrentThreadName(name);
#endif

	/* Initialize running thread. */
	running.store(true);
	LOG("Initializing thread: %s.", name);
	Initialize.Post(this, EventArgs());

	/* Run until Stop is called. */
	while (allow.load())
	{
		Tick.Post(this, EventArgs());

		/* Make sure we don't update too often. */
		if (cooldown) thread_sleep(cooldown);
	}

	/* Finalize running thread. */
	LOG("Terminating thread: %s.", name);
	Terminate.Post(this, EventArgs());
	running.store(false);
}