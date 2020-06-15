#pragma once
#include "Graphics/Color.h"
#include "Stopwatch.h"
#include <stack>

struct ImDrawList;

namespace Pu
{
	class FileWriter;
	class VulkanInstance;

	/* Defines an application global interface used to record timed events. */
	class Profiler
	{
	public:
		Profiler(_In_ const Profiler&) = delete;
		Profiler(_In_ Profiler&&) = delete;

		_Check_return_ Profiler& operator =(_In_ const Profiler&) = delete;
		_Check_return_ Profiler& operator =(_In_ Profiler&&) = delete;

		/* Starts a new recording in the profiler for the specified CPU category. */
		static void Begin(_In_ const string &category, _In_ Color color);
		/* Ends a recording for the thread specific CPU profiler. */
		static void End(void);
		/* Adds a time segment (in milliseconds) to the specified GPU category. */
		static void Add(_In_ const string &category, _In_ Color color, _In_ int64 time);
		/* Renders the current profiler data to ImGUI and clears the list. */
		static void Visualize(void);
		/* Logs the current profiler data to disk and clears the list. */
		static void Save(_In_ const wstring &path);
		/* Sets the target frame time (in seconds). */
		static void SetTargetFrameTime(_In_ float fps);
		/* Sets the smoothing interval (in seconds). */
		static void SetInterval(_In_ float value);

		/* Starts or adds to the recording of a debug specific piece of code. */
		static void BeginDebug(void)
		{
			Begin("Debug", Color::Orange());
		}

	private:
		friend class Application;
		
		using Section = std::tuple<string, Color, int64>;
		using Timer = std::pair<size_t, Stopwatch>;

		std::map<uint64, std::stack<Timer>> activeThreads;
		VulkanInstance *vkInstance;

		vector<Section> cpuSections;
		vector<Section> gpuSections;

		int64 target, ticks;
		float spacing, height, length, offset, interval;
		Stopwatch timer;

		Profiler(void);
		static Profiler& GetInstance(void);
		void BeginInternal(const string &category, Color color, uint64 thread);
		void EndInternal(uint64 thread);
		void AddInternal(const string &category, Color color, int64 time);
		void VisualizeInternal(void);
		void SaveInternal(const wstring &path);
		void ClearIfNeeded(void);
		void RenderSections(const vector<Section> &sections, const char *type, bool addDummy);
		void SaveSections(FileWriter &writer, const vector<Section> &sections);
		float DrawBarAndText(ImDrawList *drawList, float y, float x0, int64 time, Color clr, const string &txt);
		float DrawBar(ImDrawList *drawList, float y, float x0, int64 time, Color clr);
	};
}