#include "Core/Diagnostics/CPU.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/Stopwatch.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Config.h"

bool Pu::CPU::firstRun = true;
Pu::byte Pu::CPU::lastUsage = 0;
std::mutex Pu::CPU::lock;

#ifdef _WIN32
FILETIME Pu::CPU::prevSysKernel;
FILETIME Pu::CPU::prevSysUser;
FILETIME Pu::CPU::prevProcKernel;
FILETIME Pu::CPU::prevProcUser;
#endif

Pu::byte Pu::CPU::GetCurrentProcessUsage(void)
{
	lock.lock();

	/* We only need to query the CPU usage every now and then to prevent the CPU from hanging. */
	if (HasEnoughTimePassed()) QueryUsage();
	const byte result = lastUsage;

	lock.unlock();
	return result;
}

void Pu::CPU::QueryUsage()
{
#ifdef _WIN32
	/* sysIdle, procCreation and procExit aren't used but must be passed. */
	FILETIME sysIdle, sysKernel, sysUser;
	FILETIME procCreation, procExit, procKernel, procUser;

	/* Attempt to get the system timing information. */
	if (!GetSystemTimes(&sysIdle, &sysKernel, &sysUser))
	{
		Log::Error("Unable to get system times ('%ls')!", _CrtGetErrorString().c_str());
		return;
	}

	/* Attempt to get the process timing information. */
	if (!GetProcessTimes(GetCurrentProcess(), &procCreation, &procExit, &procKernel, &procUser))
	{
		Log::Error("Unable to get process times ('%s')!", _CrtGetErrorString().c_str());
		return;
	}

	if (!firstRun)
	{
		/* Calculate the difference in the previous system time and the current time. */
		const uint64 sysKernelDiff = SubtrFileTimes(sysKernel, prevSysKernel);
		const uint64 sysUserDiff = SubtrFileTimes(sysUser, prevSysUser);

		/* Calculate the difference in the previous process time and the current time. */
		const uint64 procKernelDiff = SubtrFileTimes(procKernel, prevProcKernel);
		const uint64 procUserDiff = SubtrFileTimes(procUser, prevProcUser);

		const uint64 sysTotal = sysKernelDiff + sysUserDiff;
		const uint64 procTotal = procKernelDiff - procUserDiff;

		/* Calculate the new CPU usage for the process. */
		if (sysTotal) lastUsage = static_cast<byte>((100.0f * procTotal) / sysTotal);
	}

	/* Set the previous times for next run. */
	firstRun = false;
	prevSysKernel = sysKernel;
	prevSysUser = sysUser;
	prevProcKernel = procKernel;
	prevProcUser = procUser;

#else
	Log::Warning("Querying CPU usage is not supported on this platform!");
	return 0;
#endif
}

bool Pu::CPU::HasEnoughTimePassed(void)
{
	/* Timer only needs to be available in this method, so defining it here saves an include in the header. */
	static Stopwatch timer = Stopwatch::StartNew();

	if (timer.Milliseconds() > CPUUsageQueryMinimumElapsedTime)
	{
		timer.Restart();
		return true;
	}

	return false;
}

#ifdef _WIN32
Pu::uint64 Pu::CPU::SubtrFileTimes(const FILETIME & first, const FILETIME & second)
{
	uint64 a = first.dwHighDateTime;
	a <<= 32;
	a |= first.dwLowDateTime;

	uint64 b = second.dwHighDateTime;
	b <<= 32;
	b |= second.dwLowDateTime;

	return a - b;
}
#endif