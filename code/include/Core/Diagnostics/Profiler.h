#pragma once
#include "Graphics/Color.h"
#include "Stopwatch.h"

struct ImDrawList;

namespace Pu
{
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
		/* Sets the target frame time (in seconds). */
		static void SetTargetFrameTime(_In_ float fps);
		/* Sets the smoothing interval (in seconds). */
		static void SetInterval(_In_ float value);

	private:
		using Section = std::tuple<string, Color, int64>;
		using Timer = std::pair<size_t, Stopwatch>;

		std::map<uint64, Timer> activeThreads;

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
		void RenderSections(const vector<Section> &sections, const char *type, bool addDummy);
		float DrawBarAndText(ImDrawList *drawList, float y, float x0, int64 time, Color clr, const string &txt);
		float DrawBar(ImDrawList *drawList, float y, float x0, int64 time, Color clr);
	};
}