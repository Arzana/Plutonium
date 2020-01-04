#pragma once
#include "Core/Platform/Windows/Windows.h"

#ifdef _WIN32
/* near and far are needed in the signature of the WinSock2 functions, but we don't want them. */
#ifndef near
#define near
#endif

#ifndef far
#define far
#endif

#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>

#undef near
#undef far
#endif