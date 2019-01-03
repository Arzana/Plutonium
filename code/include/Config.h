#pragma once
#include "Core/Math/Constants.h"

/*
This file is used to store compiler time constant configurations for Plutonium.
*/

namespace Pu
{
	/* Defines whether the SPIR-V compiler should display a human readable SPIR-V to the log. */
	constexpr bool SpirVCompilerLogHumanReadable = false;
	/* Defines the time (in milliseconds) before the SPIR-V compiler is shut down to prevent hanging. */
	constexpr uint64 SpirVCompilerTimeout = 2000;
	/* Defines the interval (in milliseconds) to check whether a PuThread has completed. */
	constexpr uint64 ThreadWaitSleepTime = 100;
	/* Defines the time (in milliseconds) before the PuThread wait will timeout. */
	constexpr uint64 ThreadWaitMax = 5000;
	/* Defines the interval (in milliseconds) to check whether a PuThread has started. */
	constexpr uint64 ThreadStartWaitInterval = 100;
	/* Defines whether the task scheduler is allowed to steal tasks. */
	constexpr bool TaskSchedulerStealing = true;
	/* Defines whether the event bus should log subscriber changes. */
	constexpr bool EventBusLogging = true;
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
	/* Defines whether Vulkan info messages should be logged. */
	constexpr bool LogVulkanInfoMessages = false;
	/* Defines the default number of level of detail available for minified image sampling. */
	constexpr uint32 DefaultMipLevels = 4;
}