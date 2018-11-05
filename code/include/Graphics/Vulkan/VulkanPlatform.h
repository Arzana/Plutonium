#pragma once

/*
 * Defines the calling convention per platform.
 *
 * VKAPI_ATTR - Placed before the return type in function declarations.
 *              Useful for C++11 and GCC/Clang-style function attribute syntax.
 * VKAPI_CALL - Placed after the return type in function declarations.
 *              Useful for MSVC-style calling convention syntax.
 * VKAPI_PTR  - Placed between the '(' and '*' in function pointer types.
 *
 * Function declaration:  VKAPI_ATTR void VKAPI_CALL vkCommand(void);
 * Function pointer type: using PFN_vkCommand = void(VKAPI_PTR*)(void);
*/
#if defined(_WIN32)
#define VKAPI_ATTR
#define VKAPI_CALL	__stdcall
#define VKAPI_PTR	VKAPI_CALL*
#elif defined(__ANDROID__) && defined(__ARM_EABI__) && !defined(__ARM_ARCH_7A__)
#error "Vulkan requires the 'armeabi-v7a' or 'armeabi-v7a-hard' ABI on 32-bit ARM CPUs"
#elif defined(__ANDROID__) && defined(__ARM_ARCH_7A__)
#define VKAPI_ATTR __attribute__((pcs("aapcs-vfp")))
#define VKAPI_CALL
#define VKAPI_PTR VKAPI_ATTR
#else
#define VKAPI_ATTR
#define VKAPI_CALL
#define VKAPI_PTR
#endif

#include <stddef.h>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <android/native_window.h>
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
#include <mir_toolkit/client_types.h>
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include <wayland-client.h>
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>
#endif