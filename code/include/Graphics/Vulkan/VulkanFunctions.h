#pragma once
#include "VulkanEnums.h"
#include "VulkanGlobals.h"

namespace Pu
{
	struct DebugUtilsMessengerCallbackData;

	/* Defines an application-defined memory allocation function. */
	using AllocationFunction = _Check_return_ void*(VKAPI_PTR)(_In_ void *userData, _In_ size_t size, _In_ size_t alignment, _In_ SystemAllocationScope allocationScope);
	/* Defines an application-defined memory reallocation function. */
	using ReallocationFunction = _Check_return_ void*(VKAPI_PTR)(_In_ void *userData, _In_ void *origional, size_t size, _In_ size_t alignment, _In_ SystemAllocationScope allocationScope);
	/* Defines an application-defined memory free function. */
	using FreeFunction = void(VKAPI_PTR)(_In_ void *userData, _In_ void *memory);
	/* Defines an application-defined memory allocation notification function. */
	using InternalAllocationNotification = void(VKAPI_PTR)(_In_ void *userData, _In_ size_t size, _In_ InternalAllocationType allocationType, _In_ SystemAllocationScope allocationScope);
	/* Defines an application-defined memory free notification function. */
	using InternalFreeNotification = void(VKAPI_PTR)(_In_ void *userData, _In_ size_t size, _In_ InternalAllocationType allocationType, _In_ SystemAllocationScope allocationScope);
	/* Defines a dummy function pointer type for query returns. */
	using VoidFunction = void(VKAPI_PTR)(void);
	/* Defines a callback function for debug messages. */
	using DebugUtilsMessengerCallback = _Check_return_ Bool32(VKAPI_PTR)(_In_ DebugUtilsMessageSeverityFlags messageSeverity, _In_ DebugUtilsMessageTypeFlags messageTypes, _In_ const DebugUtilsMessengerCallbackData *callbackData, _In_opt_ void *userData);
}