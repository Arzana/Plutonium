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
	/*
	Define the EAX, EBX, ECX and EDX registers.
	And query the highest extended function implemented (sets only EAX).
	*/
	int32 registers[4];
	__cpuid(registers, 0x80000000);

	/* 
	We can only query the brand string if it's supported.
	Brand string is always 48 bytes in size.
	*/
	if (registers[0] >= 0x80000004)
	{
		name.resize(48u);
		for (uint32 start = 0x80000002, i = 0; i <= 2; i++)
		{
			/* Query the 3 parts of the brand string, every part spans the entire register range. */
			__cpuid(registers, start + i);
			memcpy(name.data() + (i << 4), registers, 0x10);
		}
	}

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