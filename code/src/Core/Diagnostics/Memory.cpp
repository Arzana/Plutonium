#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <Psapi.h>
#endif

#include "Core/Diagnostics/Memory.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

using namespace Pu;

const MemoryFrame Pu::MemoryFrame::GetMemStats(void)
{
	MemoryFrame result;

#ifdef _WIN32
	/* Create storage for windows infomation. */
	MEMORYSTATUSEX info;
	info.dwLength = sizeof(MEMORYSTATUSEX);

	/* Get global memory information. */
	if (!GlobalMemoryStatusEx(&info))
	{
		string error = _CrtGetErrorString();
		Log::Warning("Unable to get global memory statistics: %s!", error.c_str());
		return result;
	}

	/* Get process memory information. */
	PROCESS_MEMORY_COUNTERS pmc;
	if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		string error = _CrtGetErrorString();
		Log::Warning("Unable to get process memory statistics: %s!", error.c_str());
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