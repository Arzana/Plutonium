#pragma once
#include <chrono>
#include "Core\Diagnostics\Logging.h"

namespace Plutonium
{
	/* Defines a helper structure for easily measuring time. */
	struct Stopwatch
	{
	public:

		/* Initializes a new instance of a stopwatch. */
		Stopwatch(void)
			: start(), end(), endCalled(false)
#if defined (DEBUG)
			, startCalled(false)
#endif
		{}

		/* Gets an already started stopwatch. */
		_Check_return_ inline static Stopwatch StartNew(void)
		{
			Stopwatch result = Stopwatch();
			result.Start();
			return result;
		}

		/* Starts the time measuring. */
		inline void Start(void)
		{
#if defined (DEBUG)
			LOG_WAR_IF(startCalled, "Calling Start on stopwatch before calling end!");
			startCalled = true;
#endif
			start = clock::now();
		}

		/* Stops the measuring. */
		inline void End(void)
		{
#if defined (DEBUG)
			LOG_WAR_IF(!startCalled, "Calling End on stopwatch before calling Start!");
#endif
			endCalled = true;
			end = clock::now();
		}

		/* Resets the stopwatch making it ready to be used again. */
		inline void Reset(void)
		{
#if defined (DEBUG)
			startCalled = false;
#endif
			endCalled = false;
			start = clock::time_point();
			end = clock::time_point();
		}

		/* Restarts the stopwatch reseting the timer and starting time measuring. */
		inline void Restart(void)
		{
#if defined (DEBUG)
			startCalled = true;
#endif
			endCalled = false;
			start = clock::now();
			end = clock::time_point();
		}

		/* Gets the amount of microseconds since End was called or now. */
		_Check_return_ inline long long Microseconds(void) const
		{
			return GetDuration<std::chrono::microseconds>();
		}

		/* Gets the amount of milliseconds since End was called or now. */
		_Check_return_ inline long long Milliseconds(void) const
		{
			return GetDuration<std::chrono::milliseconds>();
		}

		/* Gets the amount of seconds since End was called or now. */
		_Check_return_ inline long long Seconds(void) const
		{
			return GetDuration<std::chrono::seconds>();
		}

		/* Gets the amount of minutes since End was called or now. */
		_Check_return_ inline long long Minutes(void) const
		{
			return GetDuration<std::chrono::minutes>();
		}

	private:
		using clock = std::chrono::high_resolution_clock;

		clock::time_point start, end;
		bool endCalled;
#if defined (DEBUG)
		bool startCalled;
#endif

		/* Gets the amount of clock ticks since End was called or now. */
		template <typename _Ty>
		_Check_return_ inline long long GetDuration(void) const
		{
#if defined (DEBUG)
			LOG_WAR_IF(!startCalled, "Cannot get time of stopwatch that hasn't been started!");
#endif

			return std::chrono::duration_cast<_Ty>((endCalled ? end : clock::now()) - start).count();
		}
	};
}