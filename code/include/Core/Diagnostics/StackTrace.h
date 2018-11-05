#pragma once
#include "Core/String.h"
#include "Core/Math/Constants.h"
#include "Core/Collections/Vector.h"

namespace Pu
{
	/* Defines the stack information given for loggin purposes. */
	struct StackFrame
	{
	public:
		/* The name of the containing module. */
		string ModuleName;
		/* The name of the file in which the function resides. */
		string FileName;
		/* The name of the function. */
		string FunctionName;
		/* The line on which the exception was raised within the function. */
		int32 Line;

		/* Initializes an empty instance of a stack frame. */
		StackFrame(void)
			: StackFrame("Unknown", "Unknown", "Unknown", 0)
		{}
		StackFrame(_In_ const StackFrame &) = default;
		StackFrame(_In_ StackFrame &&) = default;

		_Check_return_ StackFrame& operator =(_In_ const StackFrame &) = delete;
		_Check_return_ StackFrame& operator =(_In_ StackFrame &&) = delete;

		/* Gets the specified stack frame handle. */
		_Check_return_ static uint64 GetCallerHndl(_In_ int32 framesToSkip);
		/* Attempts to get the function information from the specified frame handle. */
		_Check_return_ static bool GetCallerInfoFromHndl(_In_ uint64 hndl, _Out_ StackFrame &frame);
		/* Attempts to get the specified stack frame information. */
		_Check_return_ static bool GetCallerInfo(_In_ int32 framesToSkip, _Out_ StackFrame &frame);
		/* Attempts to get all active stack frames. */
		_Check_return_ static bool GetStackTrace(_In_ int32 framesToSkip, _Out_ vector<StackFrame> &frames);

	private:
		StackFrame(string moduleName, string file, string func, int32 line)
			: ModuleName(moduleName), FileName(file), FunctionName(func), Line(line)
		{}
	};
}