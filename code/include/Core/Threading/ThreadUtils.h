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
	/* Gets the enviroment variable with the specified name and returns their components. */
	_Check_return_ vector<string> _CrtGetEnviromentVariables(_In_ const char *name);
	/* Creates a process (checks path if not found right away) for a specified amount of time or until it finishes. */
	_Check_return_ bool _CrtRunProcess(_In_ const char *name, char *arguments, _In_ string &output , _In_opt_ uint64 timeout = maxv<uint64>());
}