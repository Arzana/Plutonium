#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <Psapi.h>
#endif

#include "Core/Diagnostics/Memory.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

using namespace Pu;

MemoryFrame Pu::MemoryFrame::GetCPUMemStats(void)
{
	MemoryFrame result;

#ifdef _WIN32
	/* Create storage for windows infomation. */
	MEMORYSTATUSEX info;
	info.dwLength = sizeof(MEMORYSTATUSEX);

	/* Get global memory information. */
	if (!GlobalMemoryStatusEx(&info))
	{
		const wstring error = _CrtGetErrorString();
		Log::Warning("Unable to get global memory statistics: %ls!", error.c_str());
		return result;
	}

	/* Get process memory information. */
	PROCESS_MEMORY_COUNTERS pmc;
	if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		const wstring error = _CrtGetErrorString();
		Log::Warning("Unable to get process memory statistics: %ls!", error.c_str());
		return result;
	}

	/* Set VRAM. */
	result.TotalVRam = info.ullTotalPageFile;
	result.UsedVRam = pmc.PagefileUsage;

	/* Set RAM. */
	result.TotalRam = info.ullTotalPhys;
	result.UsedRam = pmc.WorkingSetSize;
#else
	Log::Error("Cannot get memory statistics on this platform!");
#endif

	return result;
}

MemoryFrame Pu::MemoryFrame::GetGPUMemStats(const PhysicalDevice & physicalDevice)
{
	MemoryFrame result;
	result.TotalVRam = physicalDevice.GetDeviceLocalBytes();

	DeviceSize cur;
	if (physicalDevice.TryGetUsedDeviceLocalBytes(cur)) result.UsedVRam = cur;

	return result;
}