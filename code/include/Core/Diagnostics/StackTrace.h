#pragma once
#include <sal.h>
#include <vector>
#include "Core\Math\Constants.h"

namespace Plutonium
{
	/* Defines the stack information given for loggin purposes. */
	struct StackFrame
	{
	public:
		/* The name of the containing module. */
		const char *ModuleName;
		/* The name of the file in which the function resides. */
		const char *FileName;
		/* The name of the function. */
		const char *FunctionName;
		/* The line on which the exception was raised within the function. */
		int Line;

		/* Releases the resources allocated by the stack frame. */
		~StackFrame(void);

	private:
		friend const StackFrame* _CrtGetCallerInfoFromPtr(uint64);
		friend const StackFrame* _CrtGetCallerInfo(int32);
		friend std::vector<const StackFrame*> _CrtGetStackTrace(uint32);

		StackFrame(void)
			: ModuleName("Unknown"), FileName("Unknown"), FunctionName("Unknown"), Line(0)
		{}
	};

	/* Gets the platform specific last error in human readable format (Requires free!). */
	_Check_return_ const char* _CrtGetErrorString(void);
#if defined(_WIN32)
	/* Finalizes the processes that have been initialized. */
	void _CrtFinalizeWinProcess(void);
#endif
	/* 
	Gets the specified stack frame pointer from logging purposes.
	framesToSkip defines how many frames should be skipped before the frame snapshot should be taken.
	*/
	_Check_return_ uint64 _CrtGetCallerPtr(_In_ uint32 framesToSkip);
	/* Attempts to get the function information from the specified function address (requires delete). */
	_Check_return_ const StackFrame* _CrtGetCallerInfoFromPtr(_In_ uint64 ptr);
	/* 
	Gets the specified stack frame information for logging purposes (requires delete).
	if framesToSkip is positive it defines how many frames should be skipped before the frame snapshot should be taken.
	if framesToSkip is negative it defines how many frames should be skipped from the back before the frame snapshot should be taken.
	*/
	_Check_return_ const StackFrame* _CrtGetCallerInfo(_In_ int32 framesToSkip);
	/* 
	Gets all active stack frames (requires delete). 
	framesToSkip defines how many frames should be skipped before the frame snapshots should be taken.
	*/
	_Check_return_ std::vector<const StackFrame*> _CrtGetStackTrace(_In_ uint32 framesToSkip);
}