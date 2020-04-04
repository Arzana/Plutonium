#pragma once
#include <map>
#include "Core/Collections/sdeque.h"
#include "Core/Threading/Tasks/Task.h"
#include "Core/Threading/PuThread.h"
#include "Core/Events/UserEventArgs.h"

namespace Pu
{
	class TickThread;

	/* Defines an object that can share the load of tasks over a specified number of threads. */
	class TaskScheduler
	{
	public:
		/* Initializes a new instance of a task scheduler with a specified amount of channels. */
		TaskScheduler(_In_ size_t threadCnt = PuThread::GetMaxConcurrent() - 1);
		TaskScheduler(_In_ const TaskScheduler&) = delete;
		TaskScheduler(_In_ TaskScheduler&&) = delete;
		/* Releases the resources allocated by the scheduler. */
		~TaskScheduler(void);

		_Check_return_ TaskScheduler& operator =(_In_ const TaskScheduler&) = delete;
		_Check_return_ TaskScheduler& operator =(_In_ TaskScheduler&&) = delete;

		/* Adds a task to the scheduler, this task will be executed at an unspecified time and by an unspecified thread. */
		void Spawn(_In_ Task &task);
		/* Adds a high priority task to the scheduler, this task will be executed as soon as possible by an unspecified thread. */
		void Force(_In_ Task &task);

	private: 
		vector<TickThread*> threads;
		vector<sdeque<Task*>> tasks;
		vector<std::map<Task*, Task::Result>> waits;

		size_t ChooseThread(void) const;
		void ThreadTick(TickThread&, UserEventArgs args);
		bool ThreadTryWait(size_t idx);
		bool ThreadTryRun(size_t idx);
		bool ThreadTrySteal(size_t idx);
		void HandleTaskResult(size_t idx, Task *task, Task::Result result);
	};
}