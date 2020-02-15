﻿#include "Core/Diagnostics/Profiler.h"
#include "Core/Threading/ThreadUtils.h"
#include "imgui/include/imgui.h"

static std::mutex lock;

static inline Pu::int64 sec_to_ms(float sec)
{
	return static_cast<Pu::int64>(sec * 1000000.0f);
}

void Pu::Profiler::Begin(const string & category, Color color)
{
	lock.lock();

	/* We need the thread ID to make sure we can end the correct section afterwards. */
	GetInstance().BeginInternal(category, color, _CrtGetCurrentThreadId());

	lock.unlock();
}

void Pu::Profiler::End(void)
{
	lock.lock();

	/* We need the thread ID to make sure we end the correct section. */
	GetInstance().EndInternal(_CrtGetCurrentThreadId());

	lock.unlock();
}

void Pu::Profiler::Add(const string & category, Color color, int64 time)
{
	lock.lock();

	GetInstance().AddInternal(category, color, time);

	lock.unlock();
}

void Pu::Profiler::Visualize(void)
{
	lock.lock();
	GetInstance().VisualizeInternal();
	lock.unlock();
}

void Pu::Profiler::SetTargetFrameTime(float fps)
{
	GetInstance().target = sec_to_ms(fps);
}

Pu::Profiler::Profiler(void)
	: spacing(8.0f), length(0.05f), target(sec_to_ms(recip(60.0f)))
{
	height = ImGui::GetIO().FontGlobalScale * 10.0f;
	offset = ImGui::GetIO().FontGlobalScale * 250.0f;
}

Pu::Profiler & Pu::Profiler::GetInstance(void)
{
	static Profiler instance;
	return instance;
}

void Pu::Profiler::BeginInternal(const string & category, Color color, uint64 thread)
{
	if (activeThreads.find(thread) != activeThreads.end()) Log::Fatal("Profiler doesn't support nested sections!");

	/* Check if the category already exists, if so just start a stopwatch. */
	size_t i = 0;
	for (const auto &[cat, clr, total] : categories) 
	{
		if (cat == category)
		{
			activeThreads.emplace(thread, std::make_pair(i, Stopwatch::StartNew()));
			return;
		}

		++i;
	}

	/* The category was not found, so add it and start a new timer. */
	categories.emplace_back(std::make_tuple(category, color, 0));
	activeThreads.emplace(thread, std::make_pair(i, Stopwatch::StartNew()));
}

void Pu::Profiler::EndInternal(uint64 thread)
{
	/* Make sure that we throw if the profiler is misused. */
	decltype(activeThreads)::iterator it = activeThreads.find(thread);
	if (it == activeThreads.end()) Log::Fatal("Profiler cannot end a section that hasn't started!");

	/* Get the time and the category, then remove the entry.  */
	const int64 time = it->second.second.Microseconds();
	const size_t category = it->second.first;
	activeThreads.erase(it);

	/* Add the total time to the category list. */
	std::get<2>(categories[category]) += time;
}

void Pu::Profiler::AddInternal(const string & category, Color color, int64 time)
{
	/* Search if the category already exists. */
	for (auto &[cat, clr, total] : categories)
	{
		if (cat == category)
		{
			total += time;
			return;
		}
	}

	/* Add the category with the specified time if it doesn't exist yet. */
	categories.emplace_back(std::make_tuple(category, color, time));
}

void Pu::Profiler::VisualizeInternal(void)
{
	if (ImGui::Begin("Profiler"))
	{
		ImDrawList *gfx = ImGui::GetWindowDrawList();
		const ImVec2 start = ImGui::GetCursorScreenPos();

		float x0 = start.x + offset;
		const float maxStart = x0;

		for (const auto &[category, color, ms] : categories)
		{
			/* Render the text and bar. */
			x0 = DrawBarAndText(gfx, start.y, x0, ms, color, category);
		}

		/* Render the target framerate. */
		const float y1 = start.y + height + spacing;
		DrawBar(gfx, y1, maxStart, target, Color::White());

		ImGui::End();
	}

	categories.clear();
}

float Pu::Profiler::DrawBarAndText(ImDrawList * drawList, float y, float x0, int64 time, Color clr, const string & txt)
{
	ImGui::TextColored(clr.ToVector4(), "%s - %uus", txt.c_str(), time);
	return DrawBar(drawList, y, x0, time, clr);
}

float Pu::Profiler::DrawBar(ImDrawList * drawList, float y, float x0, int64 time, Color clr)
{
	const float x1 = x0 + time * length;
	drawList->AddRectFilled(ImVec2(x0, y), ImVec2(x1, y + height), ImColor(clr));
	return x1;
}