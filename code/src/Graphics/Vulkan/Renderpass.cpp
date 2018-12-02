#include "Graphics/Vulkan/Renderpass.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::Renderpass::Renderpass(LogicalDevice & device)
	: parent(device), hndl(nullptr), loaded(false)
{}

Pu::Renderpass::Renderpass(LogicalDevice & device, vector<Subpass>&& subpasses)
	: parent(device), subpasses(std::move(subpasses))
{
	Link();
}

Pu::Renderpass::Renderpass(Renderpass && value)
	: parent(value.parent), hndl(value.hndl), subpasses(std::move(value.subpasses)), loaded(value.IsLoaded())
{
	value.hndl = nullptr;
}

Pu::Renderpass & Pu::Renderpass::operator=(Renderpass && other)
{
	if (this != &other)
	{
		parent = std::move(other.parent);
		hndl = other.hndl;
		subpasses = std::move(other.subpasses);
		loaded.store(other.IsLoaded());

		other.hndl = nullptr;
		other.loaded.store(false);
	}

	return *this;
}

void Pu::Renderpass::Link(void)
{
	loaded.store(true);
}

Pu::Renderpass::LoadTask::LoadTask(Renderpass & result, std::initializer_list<const char*> subpasses)
	: result(result), paths(std::move(subpasses))
{}

Pu::Task::Result Pu::Renderpass::LoadTask::Execute(void)
{
	/* We need to pre add the subpassed because vector resizing created bad refrences. */
	size_t i = 0;
	for (; i < paths.size(); i++) result.subpasses.emplace_back(result.parent);

	i = 0;
	for (const char *cur : paths)
	{
		/* Create the subpass loading task */
		Subpass::LoadTask *task = new Subpass::LoadTask(result.subpasses[i++], cur);
		task->SetParent(*this);
		children.emplace_back(task);

		/* Give the task to the scheduler. */
		scheduler->Spawn(*task);
	}

	return Result();
}

Pu::Task::Result Pu::Renderpass::LoadTask::Continue(void)
{
	/* Make sure that all subpasses are loaded on debug mode. */
#ifdef _DEBUG
	for (const Subpass &cur : result.subpasses)
	{
		if (!cur.IsLoaded()) Log::Error("Not every subpass has completed loading!");
	}
#endif

	/* Delete the child tasks. */
	for (Subpass::LoadTask *subTask : children)
	{
		delete subTask;
	}

	/* Perform linking and return. */
	result.Link();
	return Result();
}