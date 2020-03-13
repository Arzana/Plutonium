#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	class PhysicalDevice;

	struct MemoryFrame
	{
	public:
		/* The total amount in bytes of virtual memory that the OS allowes. */
		uint64 TotalVRam;
		/* The amount in bytes of virtual memory used by this process. */
		uint64 UsedVRam;
		/* The total amount in bytes of physical memory that the OS allowes. */
		uint64 TotalRam;
		/* The amount in bytes of physical memory used by this process. */
		uint64 UsedRam;

		/* Initializes an empty instance of the memory frame object. */
		MemoryFrame(void)
			: TotalVRam(0), TotalRam(0), UsedVRam(0), UsedRam(0)
		{}

		/* Gets the current memory statistics for the calling process (CPU). */
		_Check_return_ static MemoryFrame GetCPUMemStats(void);
		/* Gets the current memory statictics for the calling process (GPU). */
		_Check_return_ static MemoryFrame GetGPUMemStats(_In_ const PhysicalDevice &physicalDevice);
	};
}