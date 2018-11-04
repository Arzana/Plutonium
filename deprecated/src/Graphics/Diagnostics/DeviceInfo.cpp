#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include "Graphics\Native\OpenGL.h"
#include <glad\glad.h>

#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX		0x9048

using namespace Plutonium;

uint64 gpuMemCnt = 0ULL;

const DeviceInfo * Plutonium::_CrtGetDeviceInfo(void)
{
	/* Get easy information from the OpenGL API. */
	DeviceInfo *result = new DeviceInfo();
	result->DeviceVendor = heapstr((const char*)glGetString(GL_VENDOR));
	result->DeviceConfig = heapstr((const char*)glGetString(GL_RENDERER));
	result->DriverVersion = heapstr((const char*)glGetString(GL_VERSION));
	result->ShaderVersion = heapstr((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	/*
	To get the available GPU memory we must check for extension values.
	Some NVidia cards define the GL_NVX_gpu_memory_info extension (> 195.XX drivers), we can use this to get the cards memory information.
	*/
	if (_CrtExtensionSupported("GL_NVX_gpu_memory_info"))
	{
		/* Get the total available memory in KB and convert it to bytes and store it. */
		GLint totalKb = 0;
		glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKb);
		result->FrameBufferSize = totalKb * 1000ULL;
	}

	/* Log if we could not get the total frame buffer size. */
	LOG_WAR_IF(!result->FrameBufferSize, "Could not get GPU frame buffer size from %s device!", result->DeviceVendor);
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