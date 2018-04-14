#pragma once
#include <sal.h>
#include "Core\Math\Constants.h"

namespace Plutonium
{
	/* Defines general information of the GPU. */
	struct DeviceInfo
	{
	public:
		/* The vendor of the GPU. */
		const char *DeviceVendor;
		/* The hardware configuration of the GPU's */
		const char *DeviceConfig;
		/* The version of the used driver. */
		const char *DriverVersion;
		/* The version or release number of the shading language. */
		const char *ShaderVersion;

		/* Releases the resources allocated by the info. */
		~DeviceInfo(void) noexcept;

	private:
		friend const DeviceInfo* _CrtGetDeviceInfo(void);

		DeviceInfo(void)
			: DeviceVendor(nullptr), DeviceConfig(nullptr), DriverVersion(nullptr), ShaderVersion(nullptr)
		{}
	};

	/* Gets the current information from the GPU (requires delete!). */
	_Check_return_ const DeviceInfo* _CrtGetDeviceInfo(void);
	/* Updates the memory currently used by graphical resources. */
	void _CrtUpdateUsedGPUMemory(_In_ int64 modifier);
	/* Gets the amounbt of memory currently used by graphical resources. */
	_Check_return_ uint64 _CrtGetUsedGPUMemory(void);
}