#include "Core/Threading/Tasks/Scheduler.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Threading/PuThread.h"
#include "Config.h"

Pu::TaskScheduler::TaskScheduler(size_t threadCnt)
	: stop(false)
{
	const size_t maxUsefulThreads = std::thread::hardware_concurrency();

	/* Log a warning if the thread count isn't helping anymore. */
	if (threadCnt > maxUsefulThreads)
	{
		Log::Warning("Attempting to create %zu worker threads, hardware only supports %u!", threadCnt, maxUsefulThreads);
	}

	threads.reserve(threadCnt);
	tasks.resize(threadCnt);
	waits.resize(threadCnt);
 
	/* Create the amount of threads specified. */
	for (size_t i = 0; i < threadCnt; i++)
	{
		threads.emplace_back(std::thread{ TaskScheduler::ThreadMain, this, i });
	}
}

Pu::TaskScheduler::~TaskScheduler(void)
{
	/* Signal the workers to stop and wait for them. */
	stop.store(true);
	PuThread::WaitAll(threads);
}

void Pu::TaskScheduler::Spawn(Task & task)
{
	/* Set the scheduler and push the task to the back of the lowest queue. */
	task.scheduler = this;
	tasks[ChooseThread()].push_back(&task);
}

void Pu::TaskScheduler::Force(Task & task)
{
	/* Set the scheduler and push the task to the front of the lowest queue */
	task.scheduler = this;
	tasks[ChooseThread()].push_front(&task);
}

void Pu::TaskScheduler::ThreadMain(TaskScheduler * scheduler, size_t idx)
{	
	PuThread::SetName(L"PuWrkr" + wstring::from(idx));
	if (idx < std::thread::hardware_concurrency()) PuThread::Lock(idx + 1);

	while (!scheduler->stop.load())
	{
		/* Check if any waiting tasks can continue. */
		if (scheduler->ThreadTryWait(idx)) continue;

		/*
		No tasks are waiting or no waiting task is done yet,
		so try to run a new task from our queue.
		*/
		if (scheduler->ThreadTryRun(idx)) continue;

		/*
		We have no more tasks to execute,
		so try to steal a task from another queue.
		*/
		if (scheduler->ThreadTrySteal(idx)) continue;

		/* All queues are empty so just sleep the thread to minimize CPU usage. */
		PuThread::Pause();
	}
}

size_t Pu::TaskScheduler::ChooseThread(void) const
{
	int64 lowestIdx = -1;
	size_t lowestCnt = maxv<size_t>();

	for (size_t i = 0; i < threads.size(); i++)
	{
		/* Count both queued tasks and waiting tasks. */
		const size_t curSize = tasks[i].size() + waits[i].size();
		if (curSize < lowestCnt)
		{
			lowestCnt = curSize;
			lowestIdx = static_cast<int64>(i);
		}
	}

	/* Should only occur if the user descides zero threads is enough. */
#ifdef _DEBUG
	if (lowestIdx == -1)
	{
		Log::Error("Unable to schedule task, pushing to default worker!");
		return 0;
	}
#endif

	return static_cast<size_t>(lowestIdx);
}

bool Pu::TaskScheduler::ThreadTryWait(size_t idx)
{
	/* Check if any tasks are waiting for sub tasks. */
	std::map<Task*, Task::Result> &list = waits[idx];
	if (list.empty()) return false;

	for (const auto[task, result] : list)
	{
		/* Check if the task is allowed to continue. */
		if (task->ShouldContinue())
		{
			/* Erase task for wait list and return to make sure we don't invalidate the for loop. */
			list.erase(task);

			/* The current task has no more active childs, so continue. */
			HandleTaskResult(idx, task, task->Continue());
			return true;
		}
	}

	/* No waiting taska where able to continue. */
	return false;
}

bool Pu::TaskScheduler::ThreadTryRun(size_t idx)
{
	/* Check if a task can be pulled from the back of the queue. */
	Task *task;
	if (tasks[idx].try_pop_back(task))
	{
		/* Run task. */
		HandleTaskResult(idx, task, task->Execute());
		return true;
	}

	return false;
}

bool Pu::TaskScheduler::ThreadTrySteal(size_t idx)
{
	if constexpr (TaskSchedulerStealing)
	{
		/* Loop through all other threads to see if they have tasks available. */
		for (size_t i = 0; i < threads.size(); i++)
		{
			if (i == idx) continue;

			/* Check if a task can be stolen from the front of the queue. */
			Task *task;
			if (tasks[i].try_pop_front(task))
			{
				/* Run the stolen task and return. */
				HandleTaskResult(idx, task, task->Execute());
				return true;
			}
		}
	}

	/* No task could be stolen so just return false. */
	return false;
}

void Pu::TaskScheduler::HandleTaskResult(size_t idx, Task * task, Task::Result result)
{
	/* If an immediate continuation task is set then just append that to our queue. */
	if (result.Continuation) tasks[idx].push_front(result.Continuation);

	/* If the tasks has childs that needs waiting upon, push it to the wait list. */
	if (result.Wait || task->GetChildCount() > 0) waits[idx].emplace(task, result);

	/* Mark the child as completed if needed. */
	if (task->GetChildCount() < 1 && task->parent) task->parent->MarkChildAsComplete(*task);

	/* Mark the task as completed on debug mode. */
#ifdef _DEBUG
	if (task->GetChildCount() < 1 && !result.Wait) task->completed.store(true);
#endif

	/* Delete the task if the user requested it. */
	if (result.Delete) delete task;
}