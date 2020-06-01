#define _CRT_SECURE_NO_WARNINGS

#include "Core/Diagnostics/Profiler.h"
#include "Graphics/Vulkan/CommandBuffer.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Core/Threading/ThreadUtils.h"
#include "Graphics/Vulkan/Instance.h"
#include "Core/Diagnostics/Memory.h"
#include "Core/Diagnostics/CPU.h"
#include "imgui/include/imgui.h"
#include "Streams/FileWriter.h"
#include "Physics/Systems/GJK.h"
#include <ctime>

static std::mutex lock;
static Pu::Stopwatch timer;

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

void Pu::Profiler::Save(const wstring & path)
{
	lock.lock();
	GetInstance().SaveInternal(path);
	lock.unlock();
}

void Pu::Profiler::SetTargetFrameTime(float fps)
{
	GetInstance().target = sec_to_ms(fps);
}

void Pu::Profiler::SetInterval(float value)
{
	lock.lock();
	GetInstance().interval = fabs(value);
	lock.unlock();
}

Pu::Profiler::Profiler(void)
	: spacing(8.0f), length(0.05f), target(sec_to_ms(recip(60.0f))),
	interval(1.0f), ticks(1)
{
	/* ImGui might not have been initialized. */
	if (ImGui::GetCurrentContext())
	{
		height = ImGui::GetIO().FontGlobalScale * 10.0f;
		offset = ImGui::GetIO().FontGlobalScale * 250.0f;
	}
	timer.Start();
}

Pu::Profiler & Pu::Profiler::GetInstance(void)
{
	static Profiler instance;
	return instance;
}

void Pu::Profiler::BeginInternal(const string & category, Color color, uint64 thread)
{
	/* Check if the category already exists, if so just start a stopwatch. */
	size_t i = 0;
	for (const auto &[cat, clr, total] : cpuSections)
	{
		if (cat == category)
		{
			//activeThreads.emplace(thread, std::make_pair(i, Stopwatch::StartNew()));
			break;
		}

		++i;
	}

	/* The category was not found, so add it and start a new timer. */
	if (i >= cpuSections.size()) cpuSections.emplace_back(std::make_tuple(category, color, 0));

	/* We must add a new stack if the thread hasn't recorded any data yet. */
	decltype(activeThreads)::iterator it = activeThreads.find(thread);
	if (it == activeThreads.end())
	{
		activeThreads.emplace(thread, std::stack<Timer>());
		it = activeThreads.find(thread);
	}

	/* Add the stopwatch to the thread stack. */
	it->second.push(std::make_pair(i, Stopwatch::StartNew()));
}

void Pu::Profiler::EndInternal(uint64 thread)
{
	decltype(activeThreads)::iterator it = activeThreads.find(thread);

	/* Make sure that we throw if the profiler is misused. */
#ifdef _DEBUG
	if (it == activeThreads.end() || it->second.empty())
	{
		Log::Error("Profiler cannot end a section that hasn't started!");
		return;
	}
#endif

	/* Get the time and the category, then remove the entry.  */
	const Timer &entry = it->second.top();
	const int64 time = entry.second.Microseconds();
	const size_t category = entry.first;
	it->second.pop();

	/* Add the total time to the category list. */
	std::get<2>(cpuSections[category]) += time;
}

void Pu::Profiler::AddInternal(const string & category, Color color, int64 time)
{
	/* Search if the category already exists. */
	for (auto &[cat, clr, total] : gpuSections)
	{
		if (cat == category)
		{
			total += time;
			return;
		}
	}

	/* Add the category with the specified time if it doesn't exist yet. */
	gpuSections.emplace_back(std::make_tuple(category, color, time));
}

void Pu::Profiler::VisualizeInternal(void)
{
	if (ImGui::Begin("Profiler"))
	{
		RenderSections(cpuSections, "CPU", true);
		ImGui::Separator();
		RenderSections(gpuSections, "GPU", false);

		ImGui::Separator();
		ImGui::Text("Draw Calls:             %u", CommandBuffer::GetDrawCalls());
		ImGui::Text("Bind Calls:             %u", CommandBuffer::GetBindCalls());
		ImGui::Text("Shaders Used:           %u", CommandBuffer::GetShaderCalls());
		ImGui::Text("Transfers:              %u", CommandBuffer::GetTransferCalls());
		ImGui::Text("Barriers:               %u", CommandBuffer::GetBarrierCalls());
		CommandBuffer::ResetCounters();

		ImGui::Separator();
		ImGui::Text("Collisions:			 %u", PhysicalWorld::GetCollisionCount());
		ImGui::Text("GJK Calls:              %u", GJK::GetCallCount());
		ImGui::Text("Average GJK Iterations: %u", GJK::GetAverageIterations());
		GJK::ResetCounters();

		ImGui::Separator();
		const MemoryFrame cpuMem = MemoryFrame::GetCPUMemStats();
		ImGui::Text("%s:\n%zu MB / %zu MB", CPU::GetName(), b2mb(cpuMem.UsedVRam), b2mb(cpuMem.TotalVRam));

		if (vkInstance)
		{
			for (const PhysicalDevice &device : vkInstance->GetPhysicalDevices())
			{
				const MemoryFrame gpuMem = MemoryFrame::GetGPUMemStats(device);
				ImGui::Text("%s:\n%zu MB / %zu MB\nAllocations: %u / %u",
					device.GetName(),
					b2mb(gpuMem.UsedVRam), b2mb(gpuMem.TotalVRam),
					device.GetAllocationsCount(), device.GetLimits().MaxMemoryAllocationCount);
			}
		}

		ImGui::End();
	}

	ClearIfNeeded();
}

void Pu::Profiler::SaveInternal(const wstring & path)
{
	FileWriter writer{ path };

	/* Write the timestamp to the diagnostics log. */
	const time_t now = std::time(nullptr);
	char buffer[100];
	size_t end;
	if ((end = std::strftime(buffer, sizeof(buffer), "%c", std::localtime(&now))))
	{
		writer.Write("Profiler log for: ");
		writer.Write(string::from(ticks));
		writer.Write(" tick(s), ");

		buffer[end] = '\n';
		buffer[end + 1] = '\n';
		buffer[end + 2] = '\0';
		writer.Write(buffer);
	}

	/* Write the CPU section of the profiler if anything was saved. */
	if (cpuSections.size())
	{
		writer.Write("-----------------------CPU-----------------------\n");
		SaveSections(writer, cpuSections);
	}

	/* Write the GPU section of the profiler if anything was saved. */
	if (gpuSections.size())
	{
		writer.Write("-----------------------GPU-----------------------\n");
		SaveSections(writer, gpuSections);
	}

	ClearIfNeeded();
}

void Pu::Profiler::ClearIfNeeded(void)
{
	if (timer.SecondsAccurate() >= interval)
	{
		timer.Restart();
		ticks = 1;

		/* Always clear all of the sections, to make sure that we don't overload after the window is minimized. */
		cpuSections.clear();
		gpuSections.clear();
	}
	else ticks++;
}

void Pu::Profiler::RenderSections(const vector<Section>& sections, const char * type, bool addDummy)
{
	ImGui::Text(type);

	ImDrawList *gfx = ImGui::GetWindowDrawList();
	const ImVec2 start = ImGui::GetCursorScreenPos();

	float x0 = start.x + offset;
	const float maxStart = x0;

	for (const auto &[category, color, ms] : sections)
	{
		/* Render the text and bar. */
		x0 = DrawBarAndText(gfx, start.y, x0, ms / ticks, color, category);
	}

	/* We need to add some empty space for the white bar if it's not the last bar. */
	if (addDummy) ImGui::Dummy(ImVec2(0.0f, height));

	/* Render the target framerate. */
	const float y1 = start.y + (height + spacing) * !sections.empty();
	DrawBar(gfx, y1, maxStart, target, Color::White());
}

void Pu::Profiler::SaveSections(FileWriter & writer, const vector<Section>& sections)
{
	for (const auto &[category, color, ms] : sections)
	{
		writer.Write("|- '");
		writer.Write(category);
		writer.Write("': ");
		writer.Write(string::from(ms));
		writer.Write(" ms\n");
	}
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