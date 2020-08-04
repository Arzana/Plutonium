#pragma once
#include "Core/Time.h"
#include "Core/Diagnostics/Logging.h"

namespace Pu
{
	/* Defines a helper structure for easily measuring time. */
	class Stopwatch
	{
	public:
		/* Initializes a new instance of a stopwatch. */
		Stopwatch(void)
			: start(), end(), endCalled(false)
#ifdef _DEBUG
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
#ifdef _DEBUG
			if (startCalled) Log::Warning("Calling Start on stopwatch before calling end!");
			startCalled = true;
#endif
			start = pu_now();
		}

		/* Stops the measuring. */
		inline void End(void)
		{
#ifdef _DEBUG
			if (!startCalled) Log::Warning("Calling End on stopwatch before calling Start!");
#endif
			endCalled = true;
			end = pu_now();
		}

		/* Resets the stopwatch making it ready to be used again. */
		inline void Reset(void)
		{
#ifdef _DEBUG
			startCalled = false;
#endif
			endCalled = false;
			start = pu_clock::time_point();
			end = pu_clock::time_point();
		}

		/* Restarts the stopwatch reseting the timer and starting time measuring. */
		inline void Restart(void)
		{
#ifdef _DEBUG
			startCalled = true;
#endif
			endCalled = false;
			start = pu_now();
			end = pu_clock::time_point();
		}

		/* Gets the amount of nanoseconds since End was called or now. */
		_Check_return_ inline int64 Nanoseconds(void) const
		{
			return pu_ns(start, get_end());
		}

		/* Gets the amount of microseconds since End was called or now. */
		_Check_return_ inline int64 Microseconds(void) const
		{
			return pu_us(start, get_end());
		}

		/* Gets the amount of milliseconds since End was called or now. */
		_Check_return_ inline int64 Milliseconds(void) const
		{
			return pu_ms(start, get_end());
		}

		/* Gets the amount of seconds since End was called or now. */
		_Check_return_ inline int64 Seconds(void) const
		{
			return pu_sec(start, get_end());
		}

		/* Gets the amount of seconds since End was called or now with great accuracy. */
		_Check_return_ inline float SecondsAccurate(void) const
		{
			return static_cast<float>(Nanoseconds()) * 0.000000001f;
		}

		/* Gets the amount of minutes since End was called or now. */
		_Check_return_ inline int64 Minutes(void) const
		{
			return pu_min(start, get_end());
		}

	private:
		pu_clock::time_point start, end;
		bool endCalled;
#ifdef _DEBUG
		bool startCalled;
#endif

		inline pu_clock::time_point get_end(void) const
		{
#ifdef _DEBUG
			if (!startCalled) Log::Warning("Cannot get time of stopwatch that hasn't been started!");
#endif
			return endCalled ? end : pu_now();
		}
	};
}