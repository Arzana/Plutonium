#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include <glad\glad.h>

using namespace Plutonium;

uint64 gpuMemCnt = 0LL;

const DeviceInfo * Plutonium::_CrtGetDeviceInfo(void)
{
	DeviceInfo *result = new DeviceInfo();
	result->DeviceVendor = heapstr((const char*)glGetString(GL_VENDOR));
	result->DeviceConfig = heapstr((const char*)glGetString(GL_RENDERER));
	result->DriverVersion = heapstr((const char*)glGetString(GL_VERSION));
	result->ShaderVersion = heapstr((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	return result;
}

void Plutonium::_CrtUpdateUsedGPUMemory(int64 modifier)
{
	gpuMemCnt += modifier;
}

uint64 Plutonium::_CrtGetUsedGPUMemory(void)
{
	return gpuMemCnt;
}

Plutonium::DeviceInfo::~DeviceInfo(void) noexcept
{
	free_s(DeviceVendor);
	free_s(DeviceConfig);
	free_s(DriverVersion);
	free_s(ShaderVersion);
}