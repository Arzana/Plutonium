#include "Core/Diagnostics/CPU.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/Stopwatch.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Config.h"

float Pu::CPU::lastUsage = 0.0f;
Pu::uint64 Pu::CPU::prevTotalTicks = 0;
Pu::uint64 Pu::CPU::prevIdleTicks = 0;
std::mutex Pu::CPU::lock;

float Pu::CPU::GetCurrentProcessUsage(void)
{
	lock.lock();

	/* We only need to query the CPU usage every now and then to prevent the CPU from hanging. */
	if (HasEnoughTimePassed()) QueryUsage();
	const float result = lastUsage;

	lock.unlock();
	return result;
}

void Pu::CPU::QueryUsage()
{
#ifdef _WIN32
	FILETIME idle, kernel, user;
	if (GetSystemTimes(&idle, &kernel, &user)) CalculateLoad(FileTimeToTicks(idle), FileTimeToTicks(kernel) + FileTimeToTicks(user));
	else Log::Error("Unable to retrieve system times (%ls)!", _CrtGetErrorString().c_str());
#else
	Log::Warning("Querying CPU usage is not supported on this platform!");
#endif
}

void Pu::CPU::CalculateLoad(uint64 idle, uint64 total)
{
	const uint64 diffTotal = total - prevTotalTicks;
	const uint64 diffIdle = idle - prevIdleTicks;

	lastUsage = 1.0f - (diffTotal > 0 ? (static_cast<float>(diffIdle) / diffTotal) : 0.0f);

	prevTotalTicks = total;
	prevIdleTicks = idle;
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
Pu::uint64 Pu::CPU::FileTimeToTicks(FILETIME value)
{
	ULARGE_INTEGER result;
	result.LowPart = value.dwLowDateTime;
	result.HighPart = value.dwHighDateTime;
	return result.QuadPart;
}
#endif