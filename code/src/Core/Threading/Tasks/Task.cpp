#include "Core/Threading/Tasks/Task.h"
#include "Core/Diagnostics/Logging.h"

Pu::Task::Task(void)
	: scheduler(nullptr), parent(nullptr), childCnt(0)
{}

Pu::Task::Task(Task & parent)
	: scheduler(nullptr), childCnt(0)
{
	SetParent(parent);
}

void Pu::Task::SetParent(Task & task)
{
	parent = &task;
	++task.childCnt;
}

void Pu::Task::MarkChildAsComplete(Task & child)
{
	if (child.parent != this) Log::Error("Attempting to invalidly mark child as completed!");
	else --childCnt;
}

Pu::Task::Result::Result(void)
	: Continuation(nullptr)
{}

Pu::Task::Result::Result(Task & continuation)
	: Continuation(&continuation)
{}