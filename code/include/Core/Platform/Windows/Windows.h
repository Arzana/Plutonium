#pragma once

/* Safe guard so we only try to include on windows platform. */
#ifdef _WIN32

/* 
Make sure windows doesn't define a min and max macro for us.
This breaks code as we define a type safe one ourselves.
*/
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

/* The name memory barrier is used in Vulkan so we need to undefine it if windows defines it. */
#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

/* The name create directory is used in the FileWriter so we need to undefine it if windows defines it. */
#ifdef CreateDirectory
#undef CreateDirectory

#ifdef UNICODE
#define WinCreateDirectory  CreateDirectoryW
#else
#define WinCreateDirectory  CreateDirectoryA
#endif
#endif

/* The name get current direcoty is used in the FileReader so we need to undefine it if windows defines it. */
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory

#ifdef UNICODE
#define WinGetCurrentDirectory  GetCurrentDirectoryW
#else
#define WinGetCurrentDirectory  GetCurrentDirectoryA
#endif
#endif

#endif