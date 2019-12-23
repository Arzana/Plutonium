#pragma once
#include "Core/Platform/DynamicLibLoader.h"
#include "Graphics/Vulkan/LogicalDevice.h"

struct RENDERDOC_API_1_4_0;

namespace Pu
{
	/* Defines a static helper class for accessing the RenderDoc API. */
	class RenderDoc
	{
	public:
		/* Starts the frame capture for any compatible device and any (or no) window. */
		static void StartFrameCapture(_In_ const LogicalDevice &device);
		/* Ends the frame capture for any compatible device and any (or no) window. */
		static void EndFrameCapture(_In_ const LogicalDevice &device);

	private:
		DynamicLibLoader loader;
		RENDERDOC_API_1_4_0 *api;

		RenderDoc(void);

		static RenderDoc& GetInstance(void);
	};
}