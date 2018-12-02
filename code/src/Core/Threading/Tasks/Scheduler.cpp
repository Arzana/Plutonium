#include "Core/Threading/Tasks/Scheduler.h"
#include "Core/Threading/TickThread.h"

Pu::TaskScheduler::TaskScheduler(size_t threadCnt)
{
	/* Create the amount of threads specified. */
	for (size_t i = 0; i < threadCnt; i++)
	{
		/* Set constant name. */
		string name("PuWrkr");
		name += std::to_string(i);

		/* Create worker thread. */
		TickThread *worker = new TickThread(name.c_str(), 0, void_ptr(i));
		worker->Tick.Add(*this, &TaskScheduler::ThreadTick);

		/* Push threads to buffers. */
		tasks.emplace_back(sdeque<Task*>());
		waits.emplace_back(std::map<Task*, Task::Result>());
		threads.emplace_back(worker);

		/* Start thread. */
		worker->Start();
	}
}

Pu::TaskScheduler::~TaskScheduler(void)
{
	/* Stop all threads. */
	while (!threads.empty())
	{
		TickThread *cur = threads.back();
		threads.pop_back();

		cur->StopWait();
		delete cur;
	}
}

void Pu::TaskScheduler::Spawn(Task & task)
{
	/* Set the scheduler and push the task to the lowest queue. */
	task.scheduler = this;
	tasks[ChooseThread()].push_back(&task);
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

void Pu::TaskScheduler::ThreadTick(TickThread &, UserEventArgs args)
{
	/* Get the index of the queue's associated with this thread. */
	const size_t idx = reinterpret_cast<size_t>(args.UserParam);

	/* Check if any waiting tasks can continue. */
	if (ThreadTryWait(idx)) return;

	/*
	No tasks are waiting or no waiting task is done yet,
	so try to run a new task from our queue.
	*/
	if (ThreadTryRun(idx)) return;

	/*
	We have no more tasks to execute,
	so try to steal a task from another queue.
	*/
	if (ThreadTrySteal(idx)) return;

	/* All queues are empty so just sleep the thread to minimize CPU usage. */
	PuThread::Sleep(100);
}

bool Pu::TaskScheduler::ThreadTryWait(size_t idx)
{
	/* Check if any tasks are waiting for sub tasks. */
	if (waits[idx].empty()) return false;

	for (const std::pair<Task*, Task::Result> &cur : waits[idx])
	{
		/* Check if the task still has active childs. */
		if (cur.first->GetChildCount()) continue;
		waits[idx].erase(cur.first);

		/* The current task has no more active childs, so continue. */
		HandleTaskResult(idx, cur.first, cur.first->Continue());
		return true;
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

	/* No task could be stolen so just return false. */
	return false;
}

void Pu::TaskScheduler::HandleTaskResult(size_t idx, Task * task, Task::Result result)
{
	/* If an immediate continuation task is set then just append that to our queue. */
	if (result.Continuation) tasks[idx].push_back(result.Continuation);

	/* If the tasks has childs that needs waiting upon, push it to the wait list. */
	if (task->GetChildCount() > 0) waits[idx].emplace(task, result);

	/* Mark the child as completed if needed. */
	if (task->GetChildCount() < 1 && task->parent) task->parent->MarkChildAsComplete(*task);
}