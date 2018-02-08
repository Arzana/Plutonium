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
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, nullptr);

	/* Remove newline characters from error. */
	replstr(msgBuffer, '\n', '\0');
	replstr(msgBuffer, '\r', '\0');

	/* Copy message to heap so that the user can free it platform indipendent. */
	const char *result = heapstr(msgBuffer);
	LocalFree(msgBuffer);
	return result;
#else
	LOG_WAR("Cannot get error string on this platform!");
	return "";
#endif
}

/* Logs a symbol loading exception in _CrtGetCallerInfo. */
bool firstExc = true;
void _CrtLogSymbolLoadException(const char *msg)
{
	/* Log warning header if needed. */
	if (firstExc)
	{
		LOG_WAR("Could not fully load error information!");
		firstExc = false;
	}

	/* Log current error. */
	const char *error = _CrtGetErrorString();
	LOG_WAR("%s: %s", msg, error);
	free_cstr_s(error);
}

const StackFrame Plutonium::_CrtGetCallerInfo(int framesToSkip)
{
	/* Reset loading exception handler and create default result. */
	firstExc = true;
	StackFrame result;

#if defined(_WIN32)
	/* Get caller info on windows systems. */

	/* Get current process handle. */
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	HANDLE process = GetCurrentProcess();
	if (!SymInitialize(process, nullptr, true)) _CrtLogSymbolLoadException("Could not initialize process");

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
	if (!SymFromAddr(process, address, 0, infoSymbol)) _CrtLogSymbolLoadException("Could not get function name");
	else result.FunctionName = infoSymbol->Name;

	/* Attempt to get file information from address. */
	if (!SymGetLineFromAddr64(process, address, &displ, infoFile)) _CrtLogSymbolLoadException("Could not get file information");
	else
	{
		result.FileName = infoFile->FileName;
		result.Line = infoFile->LineNumber;
	}

	/* Deallocate symbol memory and return result. */
	SymCleanup(process);
#else
	LOG_WAR("Cannot get caller information on this platform!");
#endif

	/* Reset loading exception handler and return result. */
	firstExc = true;
	return result;
}
