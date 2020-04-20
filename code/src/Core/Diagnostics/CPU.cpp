#include "Core/Diagnostics/CPU.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/Stopwatch.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Config.h"
#include <cstring>
#include <intrin.h>

float Pu::CPU::lastUsage = 0.0f;
Pu::uint64 Pu::CPU::prevTotalTicks = 0;
Pu::uint64 Pu::CPU::prevIdleTicks = 0;
std::mutex Pu::CPU::lock;
Pu::string Pu::CPU::name;

const char * Pu::CPU::GetName(void)
{
	/* We don't have to query again if it's already set. */
	if (name.length()) return name.c_str();

#ifdef _WIN32
	int32 info[4]{ -1 };
	__cpuid(info, 0x80000000);
	uint32 nExIds = info[0];

	char brandString[0x40] = { 0 };
	for (uint32 i = 0x80000000; i <= nExIds; i++)
	{
		__cpuid(info, i);

		/* The brand string is formed from 3 parts in EAX, EBX ECX and EDX. */
		if (i == 0x80000002) memcpy(brandString, info, sizeof(info));
		else if (i == 0x80000003) memcpy(brandString + 0x10, info, sizeof(info));
		else if (i == 0x80000004) memcpy(brandString + 0x20, info, sizeof(info));
	}

	name = brandString;

#else
	Log::Warning("Querying the CPU name is not suppoerted on this platform!");
#endif

	return name.c_str();
}

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