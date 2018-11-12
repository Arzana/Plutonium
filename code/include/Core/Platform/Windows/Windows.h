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

#endif