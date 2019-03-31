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

		/* Gets the CPU usage of the current process (range: [0, 100]). */
		_Check_return_ static byte GetCurrentProcessUsage(void);

	private:
		static bool firstRun;
		static byte lastUsage;
		static std::mutex lock;

#ifdef _WIN32
		static FILETIME prevSysKernel, prevSysUser;
		static FILETIME prevProcKernel, prevProcUser;

		static uint64 SubtrFileTimes(const FILETIME &first, const FILETIME &second);
#endif

		static void QueryUsage();
		static bool HasEnoughTimePassed(void);
	};
}