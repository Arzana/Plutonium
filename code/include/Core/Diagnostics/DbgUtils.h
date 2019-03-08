#pragma once
#include "Core/String.h"
#include "Core/Math/Constants.h"

#ifdef _DEBUG
#include "Graphics/Platform/NativeWindow.h"
#endif

namespace Pu
{
	/* Gets the platform specific last error in human readable format. */
	_Check_return_ wstring _CrtGetErrorString(void);

#ifdef _WIN32
	/* Converts a windows error code into a human readable format. */
	_Check_return_ wstring _CrtFormatError(uint64 error);
	/* Initializes the current process. */
	_Check_return_ bool _CrtInitializeWinProcess(void);
	/* Finalizes the processes that have been initialized. */
	void _CrtFinalizeWinProcess(void);
#endif

#ifdef _DEBUG
	/* Moves the debug terminal so it won't overlap with the specified window. */
	void _CrtMoveDebugTerminal(_In_ const NativeWindow &wnd);
#endif
}