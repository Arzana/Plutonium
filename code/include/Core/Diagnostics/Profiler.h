#pragma once
#include "Graphics/Color.h"
#include "Stopwatch.h"

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

		/* Starts a new recording in the profiler for the specified category. */
		static void Begin(_In_ const string &category, _In_ Color color);
		/* Ends a recording for the thread specific profiler. */
		static void End(void);
		/* Adds a time segment (in milliseconds) to the specified category. */
		static void Add(_In_ const string &category, _In_ Color color, _In_ int64 time);
		/* Renders the current profiler data to ImGUI and clears the list. */
		static void Visualize(void);
		/* Sets the target frame time (in seconds). */
		static void SetTargetFrameTime(_In_ float fps);

	private:
		using Section = std::tuple<string, Color, int64>;
		using Timer = std::pair<size_t, Stopwatch>;

		static int64 profilerTime;
		std::map<uint64, Timer> activeThreads;

		vector<Section> categories;
		float spacing, height, length, offset, max;

		Profiler(void);
		static Profiler& GetInstance(void);
		void BeginInternal(const string &category, Color color, uint64 thread);
		void EndInternal(uint64 thread);
		void AddInternal(const string &category, Color color, int64 time);
		void VisualizeInternal(void);
	};
}