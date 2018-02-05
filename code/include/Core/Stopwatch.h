#pragma once
#include <ctime>
#include "Core\Diagnostics\Logging.h"

namespace Plutonium
{
	/* Defines a helper structure for easily measuring time. */
	struct Stopwatch
	{
	public:
		/* Initializes a new instance of a stopwatch. */
		Stopwatch(void)
			: start(-1), end(-1)
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
			LOG_WAR_IF(start != -1, "Calling Start on stopwatch before calling end!");
			start = clock();
		}

		/* Stops the measuring. */
		inline void End(void)
		{
			LOG_WAR_IF(start == -1, "Calling End on stopwatch before calling Start!");
			end = clock();
		}

		/* Resets the stopwatch making it ready to be used again. */
		inline void Reset(void)
		{
			start = -1;
			end = -1;
		}

		/* Gets the amount of clock ticks since End was called or now. */
		inline long double Ticks(void) const
		{
			LOG_WAR_IF(start == -1, "Cannot get time of stopwatch that hasn't been started!");
			return static_cast<long double>((end == -1 ? clock() : end) - start);
		}

		/* Gets the amount of seconds since End was called or now. */
		inline long double Seconds(void) const
		{
			constexpr long double tick2second = CLOCKS_PER_SEC;
			return Ticks() / tick2second;
		}

		/* Gets the amount of minutes since End was called o now. */
		inline long double Minutes(void) const
		{
			constexpr long double tick2minute = CLOCKS_PER_SEC / 60.0L;
			return Ticks() / tick2minute;
		}

	private:
		clock_t start, end;
	};
}