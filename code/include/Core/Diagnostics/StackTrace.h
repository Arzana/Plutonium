#pragma once
#include <sal.h>

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

	private:
		friend const StackFrame _CrtGetCallerInfo(int);

		StackFrame(void)
			: FileName("Unknown"), FunctionName("Unknown"), Line(0)
		{}
	};

	/* Gets the platform specific last error in human readable format (Requires free!). */
	_Check_return_ const char* _CrtGetErrorString(void);
	/* Gets the specified stack frame information for logging purposes. */
	_Check_return_ const StackFrame _CrtGetCallerInfo(_In_ int framesToSkip);
}