#include "Core/Threading/Tasks/Task.h"
#include "Core/Diagnostics/Logging.h"

Pu::Task::Task(void)
	: parent(nullptr), childCnt(0)
{}

Pu::Task::Task(Task & parent)
	: childCnt(0)
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

Pu::Task::Result Pu::Task::Result::Default(void)
{
	return Result(nullptr, false, false);
}

Pu::Task::Result Pu::Task::Result::Continue(Task & continuation)
{
	return Result(&continuation, false, false);
}

Pu::Task::Result Pu::Task::Result::AutoDelete(void)
{
	return Result(nullptr, true, false);
}

Pu::Task::Result Pu::Task::Result::CustomWait(void)
{
	return Result(nullptr, false, true);
}

Pu::Task::Result::Result(Task * continuation, bool shouldDelete, bool shouldWait)
	: Continuation(continuation), Delete(shouldDelete), Wait(shouldWait)
{}