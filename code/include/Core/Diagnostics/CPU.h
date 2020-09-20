#pragma once
#include <mutex>
#include "Core/String.h"
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

		/* Gets the name of the currently used logical processor. */
		_Check_return_ static const char* GetName(void);
		/* Gets whether AVX instructions are supported. */
		_Check_return_ static bool SupportsAVX(void);
		/* Gets whether hyper-threading is supported. */
		_Check_return_ static bool SupportsHyperThreading(void);
		/* Gets the CPU usage of the current process. */
		_Check_return_ static float GetCurrentProcessUsage(void);
		/* Gets the amount of physical cores of this CPU. */
		_Check_return_ static uint32 GetPhysicalCoreCount(void);

	private:
		static float lastUsage;
		static uint64 prevTotalTicks;
		static uint64 prevIdleTicks;
		static std::mutex lock;
		static string name;

		static void QueryUsage();
		static void CalculateLoad(uint64 idle, uint64 total);
		static bool HasEnoughTimePassed(void);

#ifdef _WIN32
		static uint64 FileTimeToTicks(FILETIME value);
#endif
	};
}