#pragma once
#include "Core/Threading/Tasks/Task.h"

namespace Pu
{
	/* Defines a task scheduler that can execute spawned tasks on multiple threads. */
	class TaskScheduler
	{
	public:
		TaskScheduler(void) = delete;
		TaskScheduler(_In_ const TaskScheduler&) = delete;
		TaskScheduler(_In_ TaskScheduler&&) = delete;

		_Check_return_ TaskScheduler& operator =(_In_ const TaskScheduler&) = delete;
		_Check_return_ TaskScheduler& operator =(_In_ TaskScheduler&&) = delete;

		/* Adds a task to the scheduler, this task will be executed at an unspecified time and by an unspecified thread. */
		static void Spawn(_In_ Task &task);
		/* Adds a high priority task to the scheduler, this task will be executed as soon as possible by an unspecified thread. */
		static void Force(_In_ Task &task);

		/* Starts the task scheduler by creating the maximum amount of useful worker threads. */
		static void Start(void);
		/* Signals all the threads to stop execution and waits for them. */
		static void StopWait(void);
		/* Attempts to steal a task from a queue and executes that task (can only be called from a thread not created from the scheduler). */
		static void Help(void);

	private:
		static void ThreadMain(size_t idx);

		static size_t ChooseThread(void);
		static bool ThreadTryWait(size_t idx);
		static bool ThreadTryRun(size_t idx);
		static bool ThreadTrySteal(size_t idx);
		static void HandleTaskResult(size_t idx, Task *task, Task::Result result);
	};
}