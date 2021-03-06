#pragma once

/* Safe guard so we only try to include on Windows platform. */
#ifdef _WIN32

/* Required otherwise the Winsock API 1.1 is included (we need 2). */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

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

/* The name memory barrier is used in Vulkan so we need to undefine it if Windows defines it. */
#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

/* Near and far are used with x16 C++, we don't run on this and it causes compiler issues. */
#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

/* Used to switch between unicode and ASCII functions. */
#ifdef UNICODE
#define ASCII_UNICODE(ascii, unicode)	unicode	
#else
#define ASCII_UNICODE(ascii, unicode)	ascii
#endif

/* The name create directory is used in the FileWriter so we need to undefine it if Windows defines it. */
#ifdef CreateDirectory
#undef CreateDirectory
#define WinCreateDirectory				ASCII_UNICODE(CreateDirectoryA, CreateDirectoryW)
#endif

/* The name get current directory is used in the FileReader so we need to undefine it if Windows defines it. */
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#define WinGetCurrentDirectory			ASCII_UNICODE(GetCurrentDirectoryA, GetCurrentDirectoryW)
#endif

/* The name copy file is used in the FileWriter so we need to undefine it if Windows defines it. */
#ifdef CopyFile
#undef CopyFile
#define WinCopyFile						ASCII_UNICODE(CopyFileA, CopyFileW)
#endif

/* The name get environment variable is used in the thead helper sow e need to undefine it if Windows defines it. */
#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#define WinGetEnvironmentVariable		ASCII_UNICODE(GetEnvironmentVariableA, GetEnvironmentVariableW)
#endif

#endif