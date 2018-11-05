#pragma once
#include "Core/String.h"
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Gets the platform specific last error in human readable format. */
	_Check_return_ string _CrtGetErrorString(void);

#if defined(_WIN32)
	/* Converts a windows error code into a human readable format. */
	_Check_return_ string _CrtFormatError(uint64 error);
	/* Initializes the current process. */
	_Check_return_ bool _CrtInitializeWinProcess(void);
	/* Finalizes the processes that have been initialized. */
	void _CrtFinalizeWinProcess(void);
#endif
}