#include "Core/Diagnostics/Profiler.h"
#include "Core/Threading/ThreadUtils.h"
#include "imgui/include/imgui.h"

Pu::int64 Pu::Profiler::profilerTime = 0;
static std::mutex lock;

void Pu::Profiler::Begin(const string & category, Color color)
{
	Stopwatch sw = Stopwatch::StartNew();
	lock.lock();

	/* We need the thread ID to make sure we can end the correct section afterwards. */
	GetInstance().BeginInternal(category, color, _CrtGetCurrentThreadId());

	lock.unlock();
	profilerTime += sw.Milliseconds();
}

void Pu::Profiler::End(void)
{
	Stopwatch sw = Stopwatch::StartNew();
	lock.lock();

	/* We need the thread ID to make sure we end the correct section. */
	GetInstance().EndInternal(_CrtGetCurrentThreadId());

	lock.unlock();
	profilerTime += sw.Milliseconds();
}

void Pu::Profiler::Add(const string & category, Color color, int64 time)
{
	Stopwatch sw = Stopwatch::StartNew();
	lock.lock();

	GetInstance().AddInternal(category, color, time);

	lock.unlock();
	profilerTime += sw.Milliseconds();
}

void Pu::Profiler::Visualize(void)
{
	Add("Profiler", Color::Gray(), profilerTime);

	lock.lock();
	GetInstance().VisualizeInternal();
	lock.unlock();
}

void Pu::Profiler::SetTargetFrameTime(float fps)
{
	GetInstance().max = fps * 10000.0f;
}

Pu::Profiler::Profiler(void)
	: spacing(8.0f), length(10), max(recip(60.0f) * 10000.0f)
{
	height = ImGui::GetIO().FontGlobalScale * 10.0f;
	offset = ImGui::GetIO().FontGlobalScale * 200.0f;
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
	const int64 time = it->second.second.Milliseconds();
	const size_t category = it->second.first;
	activeThreads.erase(it);

	/* Add the total time to the category list. */
	std::get<2>(categories[category]) += time;
}

void Pu::Profiler::AddInternal(const string & category, Color color, int64 time)
{
	/* Search if the category already exists. */
	size_t i = 0;
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
			/* Create a random color for the bar. */
			const ImColor clr = ImColor(color.R, color.G, color.B);

			/* Render the category tag. */
			ImGui::TextColored(clr, "%s - %ums", category.c_str(), ms);

			/* Render the bar. */
			const float x1 = x0 + ms * length;
			gfx->AddRectFilled(ImVec2(x0, start.y), ImVec2(x1, start.y + height), clr);
			x0 = x1;
		}

		/* Render the target framerate. */
		const float x1 = max * length + offset;
		const float y1 = start.y + height + spacing;
		gfx->AddRectFilled(ImVec2(maxStart, y1), ImVec2(x1, y1 + height), IM_COL32_WHITE);

		/* Render the profiler controls. */
		ImGui::Dummy(ImVec2(0.0f, height * 2.0f));
		ImGui::Separator();
		ImGui::PushItemWidth(offset);

		ImGui::SliderFloat("Spacing", &spacing, 0.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Height", &height, 1.0f, 100.0f, "%.1f");
		ImGui::SliderFloat("Length", &length, 5.0f, 100.0f, "%.1f");
		ImGui::PopItemWidth();

		ImGui::End();
	}

	categories.clear();
	profilerTime = 0;
}