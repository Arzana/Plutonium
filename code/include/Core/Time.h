#pragma once
#include <chrono>
#include "Math/Constants.h"

namespace Pu
{
	/* Defines the clock used by the Plutonium time functions. */
	using pu_clock = std::chrono::high_resolution_clock;

	/* Gets the current time point. */
	_Check_return_ static inline pu_clock::time_point pu_now(void)
	{
		return pu_clock::now();
	}

	/* Gets the duraction between a and b in the specified duration type. */
	template <typename duration_t>
	_Check_return_ static inline auto pu_time_diff(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return std::chrono::duration_cast<duration_t>(b - a).count();
	}

	/* Gets the amount of nanoseconds between a and b. */
	_Check_return_ static inline int64 pu_ns(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return pu_time_diff<std::chrono::nanoseconds>(a, b);
	}

	/* Gets the amount of microseconds between a and b. */
	_Check_return_ static inline int64 pu_us(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return pu_time_diff<std::chrono::microseconds>(a, b);
	}

	/* Gets the amount of milliseconds between a and b. */
	_Check_return_ static inline int64 pu_ms(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return pu_time_diff<std::chrono::milliseconds>(a, b);
	}

	/* Gets the amount of seconds between a and b. */
	_Check_return_ static inline int64 pu_sec(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return pu_time_diff<std::chrono::seconds>(a, b);
	}

	/* Gets the amount of minutes between a and b. */
	_Check_return_ static inline int32 pu_min(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return pu_time_diff<std::chrono::minutes>(a, b);
	}

	/* Gets the amount of hours between a and b. */
	_Check_return_ static inline int32 pu_hour(_In_ pu_clock::time_point a, _In_ pu_clock::time_point b)
	{
		return pu_time_diff<std::chrono::hours>(a, b);
	}
}