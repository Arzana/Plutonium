#include "Core/Diagnostics/CPU.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/Stopwatch.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Config.h"
#include <cstring>
#include <intrin.h>

#define CPUID_OPCODE_MANUFACTURER_ID		0x0
#define CPUID_OPCODE_PROCESSOR_INFO			0x1
#define CPUID_OPCODE_HIGHEST_FUNCTION		0x80000000
#define CPUID_OPCODE_BRAND_STRING_START		0x80000002
#define CPUID_OPCODE_BRAND_STRING_END		0x80000004

float Pu::CPU::lastUsage = 0.0f;
Pu::uint64 Pu::CPU::prevTotalTicks = 0;
Pu::uint64 Pu::CPU::prevIdleTicks = 0;
std::mutex Pu::CPU::lock;
Pu::string Pu::CPU::name;

union cpuid_registers
{
	Pu::int32 param[4];

#pragma warning(push)
#pragma warning(disable:4201)
	struct
	{
		Pu::uint32 EAX;
		Pu::uint32 EBX;
		Pu::uint32 ECX;
		Pu::uint32 EDX;
	};
#pragma warning (pop)
};

constexpr Pu::int32 cpuid_mask(Pu::int32 mask)
{
	return 1 << (mask - 1);
}

const char * Pu::CPU::GetName(void)
{
	/* We don't have to query again if it's already set. */
	if (name.length()) return name.c_str();

	/*
	Define the EAX, EBX, ECX and EDX registers.
	And query the highest extended function implemented (sets only EAX).
	*/
	cpuid_registers registers;
	__cpuid(registers.param, CPUID_OPCODE_HIGHEST_FUNCTION);

	/* 
	We can only query the brand string if it's supported.
	Brand string is always 48 bytes in size.
	*/
	if (registers.EAX >= CPUID_OPCODE_BRAND_STRING_END)
	{
		name.resize(48u);
		for (uint32 start = CPUID_OPCODE_BRAND_STRING_START, i = 0; i <= 2; i++)
		{
			/* Query the 3 parts of the brand string, every part spans the entire register range. */
			__cpuid(registers.param, start + i);
			memcpy(name.data() + (i << 4), registers.param, 0x10);
		}
	}
	else
	{
		/* The brand string is not supported on this CPU, use the manufacturer ID instead. */
		__cpuid(registers.param, 0x0);

		/* Brand string is 3-words or 12-bytes in size and is stored in EBX, EDX, ECX (in that order). */
		name.resize(12u);
		memcpy(name.data(), &registers.EBX, sizeof(uint32));
		memcpy(name.data() + 4, &registers.EDX, sizeof(uint32));
		memcpy(name.data() + 8, &registers.ECX, sizeof(uint32));
	}

	return name.c_str();
}

bool Pu::CPU::SupportsAVX(void)
{
	/* AVX support is in the 28th-bit of the feature bits ECX (CPUID 1). */
	constexpr int32 avx_mask = cpuid_mask(28);

	cpuid_registers registers;
	__cpuid(registers.param, CPUID_OPCODE_PROCESSOR_INFO);
	return registers.ECX & avx_mask;
}

bool Pu::CPU::SupportsHyperThreading(void)
{
	/* Hyper-threading support is in the 28th-bit of the feature bits EDX (CPUID 1) */
	constexpr int32 htt_mask = cpuid_mask(28);

	cpuid_registers registers;
	__cpuid(registers.param, CPUID_OPCODE_PROCESSOR_INFO);
	return registers.EDX & htt_mask;
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

Pu::uint32 Pu::CPU::GetPhysicalCoreCount(void)
{
	/* 
	Hardware concurrency returns the amount of logical processors.
	This is mappen 1-1 to physical cores on older systems, 
	but not when hyper-threading is enabled (which is most modern systems).
	So we divide by 2 is hyper-threading is enabled.
	*/
	return std::thread::hardware_concurrency() >> static_cast<uint32>(SupportsHyperThreading());
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