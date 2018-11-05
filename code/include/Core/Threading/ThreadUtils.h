#pragma once
#include "Core/String.h"
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Gets the current process ID. */
	_Check_return_ uint64 _CrtGetCurrentProcessId(void);
	/* Gets the module name of the specified process. */
	_Check_return_ string _CrtGetProcessNameFromId(_In_ uint64 id);
	/* Gets the current thread ID. */
	_Check_return_ uint64 _CrtGetCurrentThreadId(void);
	/* Gets the description of the specified thread; if no description is set return the creators module name. */
	_Check_return_ string _CrtGetThreadNameFromId(_In_ uint64 id);
	/* Gets the description of the current thread; if no description is set return the creators module name. */
	_Check_return_ inline string _CrtGetCurrentThreadName(void)
	{
		return _CrtGetThreadNameFromId(_CrtGetCurrentThreadId());
	}
	/* Sets the current thread's description. */
	void _CrtSetCurrentThreadName(_In_ const char *name);
}