#pragma once
#include <atomic>
#include "Core/Collections/vector.h"

namespace Pu
{
	class TaskScheduler;

	/* Defines a single task that can be executed at any time. */
	class Task
	{
	public:
		/* Defines the result of a task execution. */
		struct Result
		{
		public:
			/* Specifies that this task should run immediately. */
			Task* Continuation;

			/* Initializes an empty instance of the task result object. */
			Result(void);
			/* Initializes a task result as a continuation result. */
			Result(_In_ Task &continuation);
		};

		Task(_In_ Task&&) = delete;

		_Check_return_ Task& operator =(_In_ const Task&) = delete;
		_Check_return_ Task& operator =(_In_ Task&&) = delete;

		/* Execute the task. */
		_Check_return_ virtual Result Execute(void) = 0;
		/* Continues an already executed task. */
		_Check_return_ virtual Result Continue(void)
		{
			return Result();
		}

		/* Gets the amount of childs this tasks has ative, a task will not be deleted before all childs are deleted. */
		_Check_return_ inline size_t GetChildCount(void) const
		{
			return childCnt.load();
		}

	protected:
		friend class TaskScheduler;

		/* Defines the scheduler that ran the task. */
		TaskScheduler *scheduler;
		/* Specifies the parent task. */
		Task* parent;

		Task(void);
		Task(_In_ Task &parent);

		void MarkChildAsComplete(_In_ Task &child);

	private:
		std::atomic_size_t childCnt;
	};
}