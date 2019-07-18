#pragma once
#include "Core/Math/Constants.h"

/*
This file is used to store compiler time constant configurations for Plutonium.
*/

namespace Pu
{
	/* Defines the interval (in milliseconds) to check whether a PuThread has completed. */
	constexpr uint64 ThreadWaitSleepTime = 100;
	/* Defines the time (in milliseconds) before the PuThread wait will timeout. */
	constexpr uint64 ThreadWaitMax = 5000;
	/* Defines the interval (in milliseconds) to check whether a PuThread has started. */
	constexpr uint64 ThreadStartWaitInterval = 100;
	/* Defines whether the task scheduler is allowed to steal tasks. */
	constexpr bool TaskSchedulerStealing = true;
	/* Defines whether the event bus should log subscriber changes and posts. */
	constexpr bool EventBusLogging = false;
	/* Defines whether the logger should display external code in stack traces. */
	constexpr bool LoggerExternalsVisible = false;
	/* Defines whether the available extensions and layers should be logged by the Vulkan instance. */
	constexpr bool LogAvailableVulkanExtensionsAndLayers = true;
	/* Defines the default target frame rate (in seconds) for a focused application. */
	constexpr float ApplicationFocusedTargetTime = 1.0f / 60.0f;
	/* Defines the default target frame rate (in seconds) for a non-focused application. */
	constexpr float ApplicationNoFocusTargetTime = 1.0f / 20.0f;
	/* Defines the maxiumum amount of update lag (in seconds) an application is allowed to catch up on. */
	constexpr float ApplicationMaxLagCompensation = 5.0f;
	/* Defines whether Vulkan verbose messages should be logged. */
	constexpr bool LogVulkanVerboseMessages = false;
	/* Defines whether Vulkan info messages should be logged. */
	constexpr bool LogVulkanInfoMessages = false;
	/* Defines the default number of level of detail available for minified image sampling. */
	constexpr uint32 DefaultMipLevels = 4;
	/* Defines the initial amount of command buffer allocated for asset loading and saving. */
	constexpr size_t InitialLoadCommandBufferCount = 2;
	/* Defines the minimum loadable GLTF version. */
	constexpr float MinimumVersionGLTF = 2.0f;
	/* Defines the preferred amount of components when loading images (0 mean use native components). */
	constexpr int PreferredImageComponentCount = 4;
	/* Defines the time (in milliseconds) before the CPU usage is allowed to be queried again. */
	constexpr int64 CPUUsageQueryMinimumElapsedTime = 1000;
	/* Defines whether images loaded from disk should be allowed to be saved to disk. */
	constexpr bool AllowSaveOnLoadedImages = false;
	/* Defines the horizontal offset used within font atlases between glyphs (lower values mean less memory but more chance of corruption). */
	constexpr uint32 FontAtlasHOffset = 2;
	/* Defines the vertical offset used within font atlases between glyphs (lower values mean less memory but more chance of corruption). */
	constexpr uint32 FontAtlasVOffset = 2;
	/* Defines whether to log a fatal exception on Vulkan validation errors instead of just loggin it. */
	constexpr bool VulkanRaiseOnError = true;
	/* Defines whether ImGui should be available. */
	constexpr bool ImGuiAvailable = true;
	/* Defines the maximum amount of vertices that can be rendered in one frame of the debug renderer. */
	constexpr size_t MaxDebugRendererVertices = 4096;
	/* Defines whether to log warnings about wasted GPU memory due to allignment or minimum buffer sizes. */
	constexpr bool LogWastedMemory = false;
}