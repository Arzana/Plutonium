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

/* Force unicode throughout the Windows API. */
#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>

/* The name memory barrier is used in Vulkan so we need to undefine it if windows defines it. */
#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

/* Used to switch between unicode and ASCII functions. */
#ifdef UNICODE
#define ASCII_UNICODE(ascii, unicode)	unicode	
#else
#define ASCII_UNICODE(ascii, unicode)	ascii
#endif

/* The name create directory is used in the FileWriter so we need to undefine it if windows defines it. */
#ifdef CreateDirectory
#undef CreateDirectory
#define WinCreateDirectory				ASCII_UNICODE(CreateDirectoryA, CreateDirectoryW)
#endif

/* The name get current direcoty is used in the FileReader so we need to undefine it if windows defines it. */
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#define WinGetCurrentDirectory			ASCII_UNICODE(GetCurrentDirectoryA, GetCurrentDirectoryW)
#endif

#endif