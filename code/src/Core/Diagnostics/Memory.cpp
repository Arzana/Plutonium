#include "Core\Diagnostics\Memory.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Core\SafeMemory.h"

#if defined(_WIN32)
/* Include Windows specific debug headers. */
#include <Windows.h>
#include <Psapi.h>
#endif

using namespace Plutonium;

const MemoryFrame Plutonium::_CrtGetMemStats(void)
{
	MemoryFrame result = MemoryFrame();

#if defined(_WIN32)
	/* Create storage for windows infomation. */
	MEMORYSTATUSEX info;
	info.dwLength = sizeof(MEMORYSTATUSEX);
	PROCESS_MEMORY_COUNTERS pmc;

	/* Get global memory information. */
	if (!GlobalMemoryStatusEx(&info))
	{
		const char *error = _CrtGetErrorString();
		LOG_WAR("Unable to get global memory statistics: %s!", error);
		free_cstr_s(error);
		return result;
	}

	/* Get process memory information. */
	if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		const char *error = _CrtGetErrorString();
		LOG_WAR("Unable to get process memory statistics: %s!", error);
		free_cstr_s(error);
		return result;
	}

	/* Set VRAM. */
	result.TotalVRam = info.ullTotalPageFile;
	result.UsedVRam = pmc.PagefileUsage;

	/* Set RAM. */
	result.TotalRam = info.ullTotalPhys;
	result.UsedRam = pmc.WorkingSetSize;

#else
	LOG_WAR("Cannot get memory statistics on this platform!");
#endif

	return result;
}