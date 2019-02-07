#pragma once
#include <atomic>
#include "Core/Collections/vector.h"
#include "Core/Diagnostics/Logging.h"

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
			/* Specifies whether the task should be deleted upon completion. */
			bool Delete;
			/* Specifies whether the task needs to be put into a wait list, this is done automatically if it has childs. */
			bool Wait;

			/* Gets a default result, nothing will happen after this. */
			_Check_return_ static Result Default(void);
			/* Gets a new task result as a continue task. */
			_Check_return_ static Result Continue(_In_ Task &continuation);
			/* Gets a new task result as a auto delete result. */
			_Check_return_ static Result AutoDelete(void);
			/* Gets a new task result as a custom wait result. */
			_Check_return_ static Result CustomWait(void);

		private:
			Result(Task *continuation, bool shouldDelete, bool shouldWait);
		};

		Task(_In_ Task&&) = delete;
		/* Releases the resources associated with the task. */
		virtual ~Task(void) 
		{
#ifdef _DEBUG
			if (!completed.load()) Log::Warning("Task is being destroyed before being completed!");
#endif
		}

		_Check_return_ Task& operator =(_In_ const Task&) = delete;
		_Check_return_ Task& operator =(_In_ Task&&) = delete;

		/* Execute the task. */
		_Check_return_ virtual Result Execute(void) = 0;
		/* Continues an already executed task. */
		_Check_return_ virtual Result Continue(void)
		{
			return Result::Default();
		}

		/* Gets the amount of childs this tasks has ative, a task will not be deleted before all childs are deleted. */
		_Check_return_ inline size_t GetChildCount(void) const
		{
			return childCnt.load();
		}

		/* Sets the parent task of this task. */
		void SetParent(_In_ Task &task);

	protected:
		friend class TaskScheduler;

		/* Defines the scheduler that ran the task. */
		TaskScheduler *scheduler;
		/* Specifies the parent task. */
		Task* parent;

		/* Initializes an empty instance of a task. */
		Task(void);
		/* Initializes a task as a child of another task. */
		Task(_In_ Task &parent);

		/* Marks a child task as complete, used mainly by the scheduler. */
		void MarkChildAsComplete(_In_ Task &child);

		/* Checks whether the task can continue, this can be overwritten by tasks for custom usage. */
		_Check_return_ virtual inline bool ShouldContinue(void) const 
		{
			return GetChildCount() <= 0;
		}

	private:
		std::atomic_size_t childCnt;
#ifdef _DEBUG
		std::atomic_bool completed;
#endif
	};
}