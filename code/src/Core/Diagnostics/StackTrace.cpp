#include "Core\Diagnostics\StackTrace.h"
#include "Core\Math\Constants.h"
#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"

#if defined(_WIN32)
/* Include windows specific debug headers. */
#include <Windows.h>
#include <DbgHelp.h>
#endif

using namespace Plutonium;

const char * Plutonium::_CrtGetErrorString(void)
{
#if defined(_WIN32)
	/* Get error string on windows systems.*/

	/* Get error underlying error code and early out if no error was raised. */
	DWORD error = GetLastError();
	if (error == NO_ERROR) return "";

	/* Get human readable error from system. */
	LPSTR msgBuffer = nullptr;
	size_t len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							   nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, nullptr);

	/* Copy message to heap so that the user can free it platform indipendent. */
	const char *result = heapstr(msgBuffer);
	LocalFree(msgBuffer);
	return result;
#else
	LOG_WAR("Cannot get error string on this platform!");
	return "";
#endif
}

const StackFrame Plutonium::_CrtGetCallerInfo(int framesToSkip)
{
#if defined(_WIN32)
	/* Get caller info on windows systems. */
	StackFrame result;

	/* Get current process handle. */
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, nullptr, true);

	/* Initialize symbol info. */
	SYMBOL_INFO *infoSymbol = reinterpret_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)));
	infoSymbol->MaxNameLen = MAX_SYM_NAME;
	infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	/* Initialize file info. */
	IMAGEHLP_LINE64 *infoFile = malloc_s(IMAGEHLP_LINE64, 1);
	infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	/* We don't use displacement but the function needs it. */
	DWORD displ;

	/* Get required stack frame. */
	void *frames[1024];
	uint16 frameCnt = CaptureStackBackTrace(framesToSkip, MAXUINT16, frames, nullptr);
	uint64 address = reinterpret_cast<uint64>(*frames);

	/* Attempt to get symbol information from address. */
	if (!SymFromAddr(process, address, 0, infoSymbol))
	{
		const char *error = _CrtGetErrorString();
		LOG_WAR("Could not get symbol information: %s!", error);
		free_cstr_s(error);
	}
	else result.FunctionName = infoSymbol->Name;

	/* Attempt to get file information from address. */
	if (!SymGetLineFromAddr64(process, address, &displ, infoFile))
	{
		const char *error = _CrtGetErrorString();
		LOG_WAR("Could not get file information: %s!", error);
		free_cstr_s(error);
	}
	else
	{
		result.FileName = infoFile->FileName;
		result.Line = infoFile->LineNumber;
	}

	return result;
#else
	LOG_WAR("Cannot get caller information on this platform!");
	return StackFrame();
#endif
}
