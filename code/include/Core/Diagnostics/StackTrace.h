#pragma once
#include <sal.h>
#include "Core\Math\Constants.h"

namespace Plutonium
{
	/* Defines the information given for loggin purposes. */
	struct StackFrame
	{
	public:
		/* The name of the file in which the function resides. */
		const char *FileName;
		/* The name of the function. */
		const char *FunctionName;
		/* The line on which the exception was raised within the function. */
		int Line;

		/* Releases the resources allocated by the stack frame. */
		~StackFrame(void);

	private:
		friend const StackFrame _CrtGetCallerInfoFromPtr(uint64);
		friend const StackFrame _CrtGetCallerInfo(int);

		StackFrame(void)
			: FileName("Unknown"), FunctionName("Unknown"), Line(0)
		{}
	};

	/* Gets the platform specific last error in human readable format (Requires free!). */
	_Check_return_ const char* _CrtGetErrorString(void);
	/* Gets the specified stack frame pointer from logging purposes. */
	_Check_return_ uint64 _CrtGetCallerPtr(_In_ int framesToSkip);
	/* Attempts to get the function information from the specified function address. */
	_Check_return_ const StackFrame _CrtGetCallerInfoFromPtr(_In_ uint64 ptr);
	/* Gets the specified stack frame information for logging purposes. */
	_Check_return_ const StackFrame _CrtGetCallerInfo(_In_ int framesToSkip);
}