#pragma once
#include "Core\Math\Constants.h"

namespace Plutonium
{
	/* Gets the current process ID. */
	_Check_return_ uint64 _CrtGetCurrentProcessId(void);
	/* Gets the module name of the specified process (requires free!). */
	_Check_return_ const char* _CrtGetProcessNameFromId(_In_ uint64 id);
	/* Gets the current thread ID. */
	_Check_return_ uint64 _CrtGetCurrentThreadId(void);
	/* Gets the description of the specified thread; if no description is set return the creators module name (requires free!). */
	_Check_return_ const char* _CrtGetThreadNameFromId(_In_ uint64 id);
	/* Gets the description of the current thread; if no description is set return the creators module name (requires free!). */
	_Check_return_ inline const char* _CrtGetCurrentThreadName(void)
	{
		return _CrtGetThreadNameFromId(_CrtGetCurrentThreadId());
	}
	/* Sets the current thread's description. */
	void _CrtSetCurrentThreadName(_In_ const char *name);
}