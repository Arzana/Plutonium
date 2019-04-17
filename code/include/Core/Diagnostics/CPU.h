#pragma once
#include <mutex>
#include "Core/Math/Constants.h"
#include "Core/Platform/Windows/Windows.h"

namespace Pu
{
	/* Defines a static helper object for getting CPU information. */
	class CPU
	{
	public:
		CPU(void) = delete;
		CPU(_In_ const CPU&) = delete;
		CPU(_In_ CPU&&) = delete;

		_Check_return_ CPU& operator =(_In_ const CPU&) = delete;
		_Check_return_ CPU& operator =(_In_ CPU&&) = delete;

		/* Gets the CPU usage of the current process. */
		_Check_return_ static float GetCurrentProcessUsage(void);

	private:
		static bool firstRun;
		static float lastUsage;
		static uint64 prevTotalTicks;
		static uint64 prevIdleTicks;
		static std::mutex lock;

		static void QueryUsage();
		static void CalculateLoad(uint64 idle, uint64 total);
		static bool HasEnoughTimePassed(void);

#ifdef _WIN32
		static uint64 FileTimeToTicks(FILETIME value);
#endif
	};
}