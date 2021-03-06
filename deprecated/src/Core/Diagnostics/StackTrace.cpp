#include "Core\Diagnostics\StackTrace.h"
#include "Core\Math\Constants.h"
#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include <vector>

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
	if (error == NO_ERROR) return heapstr("No Error!");

	return _CrtFormatError(error);
#else
	LOG_WAR_ONCE("Cannot get error string on this platform!");
	return "";
#endif
}

/* Logs a symbol loading exception in _CrtGetCallerInfo. */
bool firstExc = true;
void _CrtLogSymbolLoadException(uint64 address, const char *msg)
{
	/* Log warning header if needed. */
	if (firstExc)
	{
		LOG_WAR("Could not fully load error information!");
		firstExc = false;
	}

	/* Log current error. */
	const char *error = _CrtGetErrorString();
	if (address) LOG_WAR("- %s %#016x: %s", msg, address, error);
	else LOG_WAR("- %s: %s", msg, error);
	free_s(error);
}

#if defined (_WIN32)
std::vector<HANDLE> initializedProcesses;

void InitializeProcess(HANDLE process)
{
	/* Check if process has already been initialized. */
	for (size_t i = 0; i < initializedProcesses.size(); i++)
	{
		if (initializedProcesses.at(i) == process) return;
	}

	/* Initialize process and add it to the list. */
	initializedProcesses.push_back(process);
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	if (!SymInitialize(process, nullptr, true)) _CrtLogSymbolLoadException(0L, "Could not initialize process");
	else LOG("Initialized debug symbols for process %lu.", GetProcessId(process));
}

const char * Plutonium::_CrtFormatError(uint64 error)
{
	/* Get human readable error from system. */
	LPSTR msgBuffer = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, static_cast<DWORD>(error), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, nullptr);

	/* Remove newline characters from error. */
	replstr(msgBuffer, '\n', '\0');
	replstr(msgBuffer, '\r', '\0');

	/* Copy message to heap so that the user can free it platform indipendent. */
	const char *result = heapstr(msgBuffer);
	LocalFree(msgBuffer);
	return result;
}

void Plutonium::_CrtFinalizeWinProcess(void)
{
	LOG("Finalizing debug symbols for %u process(es).", initializedProcesses.size());
	while (initializedProcesses.size() > 0)
	{
		SymCleanup(initializedProcesses.back());
		initializedProcesses.pop_back();
	}
}
#endif

uint64 Plutonium::_CrtGetCallerPtr(uint32 framesToSkip)
{
	/* Reset loading exception handler and create default result. */
	firstExc = true;
	uint64 result;

#if defined(_WIN32)
	/* Get caller info on windows systems. */

	/* Get current process handle. */
	HANDLE process = GetCurrentProcess();
	InitializeProcess(process);

	/* Get required stack frame. */
	void *frames[1];
	CaptureStackBackTrace(framesToSkip, 1, frames, nullptr);
	result = reinterpret_cast<uint64>(frames[0]);
#else
	LOG_WAR_ONCE("Cannot get caller address on this platform!");
#endif

	/* Reset loading exception handler and return result. */
	firstExc = true;
	return result;
}

const StackFrame* Plutonium::_CrtGetCallerInfoFromPtr(uint64 ptr)
{
	/* Reset loading exception handler and create default result. */
	firstExc = true;
	StackFrame *result = new StackFrame();

#if defined(_WIN32)
	/* Get caller info on windows systems. */

	/* Get current process handle. */
	HANDLE process = GetCurrentProcess();
	InitializeProcess(process);

	/* Initialize symbol info. */
	SYMBOL_INFO *infoSymbol = reinterpret_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)));
	infoSymbol->MaxNameLen = MAX_SYM_NAME;
	infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	/* Initialize file info. */
	IMAGEHLP_LINE64 *infoFile = malloc_s(IMAGEHLP_LINE64, 1);
	infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	/* Initialize the module info. */
	IMAGEHLP_MODULE64 *infoModule = malloc_s(IMAGEHLP_MODULE64, 1);
	infoModule->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

	/* We don't use displacement but the function needs it. */
	DWORD displ;

	/* Attempt to get symbol information from address. */
	if (!SymFromAddr(process, ptr, 0, infoSymbol)) _CrtLogSymbolLoadException(ptr, "Could not get function name for");
	else result->FunctionName = heapstr(infoSymbol->Name);

	/* Attempt to get file information from address. */
	if (!SymGetLineFromAddr64(process, ptr, &displ, infoFile)) _CrtLogSymbolLoadException(ptr, "Could not get file information for");
	else
	{
		result->FileName = heapstr(infoFile->FileName);
		result->Line = infoFile->LineNumber;
	}

	/* Attempt to get the module information from address. */
	if (!SymGetModuleInfo64(process, ptr, infoModule)) _CrtLogSymbolLoadException(ptr, "Could not get module info for");
	else result->ModuleName = heapstr(infoModule->ModuleName);

	free_s(infoSymbol);
	free_s(infoFile);
	free_s(infoModule);
#else
	LOG_WAR_ONCE("Cannot get caller information on this platform!");
#endif

	/* Reset loading exception handler and return result. */
	firstExc = true;
	return result;
}

const StackFrame* Plutonium::_CrtGetCallerInfo(int32 framesToSkip)
{
	/* Reset loading exception handler and create default result. */
	firstExc = true;
	StackFrame *result = new StackFrame();

#if defined(_WIN32)
	/* Get caller info on windows systems. */

	/* Get current process handle. */
	HANDLE process = GetCurrentProcess();
	InitializeProcess(process);

	/* Initialize symbol info. */
	SYMBOL_INFO *infoSymbol = reinterpret_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)));
	infoSymbol->MaxNameLen = MAX_SYM_NAME;
	infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	/* Initialize file info. */
	IMAGEHLP_LINE64 *infoFile = malloc_s(IMAGEHLP_LINE64, 1);
	infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	/* Initialize the module info. */
	IMAGEHLP_MODULE64 *infoModule = malloc_s(IMAGEHLP_MODULE64, 1);
	infoModule->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

	/* We don't use displacement but the function needs it. */
	DWORD displ;

	/* Get required stack frame. */
	void *frames[32];
	uint16 frameCnt = CaptureStackBackTrace(framesToSkip > 0 ? framesToSkip : 0, 32, frames, nullptr);

	/* Loop through frames untill a valid frame is found. */
	bool hasFunc = false, hasFile = false, hasModule = false;
	for (size_t i = framesToSkip >= 0 ? 0 : frameCnt + framesToSkip; i < frameCnt && !(hasFunc || hasFile || hasModule); i++)
	{
		uint64 address = reinterpret_cast<uint64>(frames[i]);
		
		/* Attempt to get symbol information from address. */
		if (!hasFunc)
		{
			if (!SymFromAddr(process, address, 0, infoSymbol)) _CrtLogSymbolLoadException(address, "Could not get function name for");
			else
			{
				hasFunc = true;
				result->FunctionName = heapstr(infoSymbol->Name);
			}
		}

		/* Attempt to get file information from address. */
		if (!hasFile)
		{
			if (!SymGetLineFromAddr64(process, address, &displ, infoFile)) _CrtLogSymbolLoadException(address, "Could not get file information for");
			else
			{
				hasFile = true;
				result->FileName = heapstr(infoFile->FileName);
				result->Line = infoFile->LineNumber;
			}
		}

		/* Attempt to get the module information from address. */
		if (!hasModule)
		{
			if (!SymGetModuleInfo64(process, address, infoModule)) _CrtLogSymbolLoadException(address, "Could not get module info for");
			else
			{
				hasModule = true;
				result->ModuleName = heapstr(infoModule->ModuleName);
			}
		}
	}

	free_s(infoSymbol);
	free_s(infoFile);
	free_s(infoModule);
#else
	LOG_WAR_ONCE("Cannot get caller information on this platform!");
#endif

	/* Reset loading exception handler and return result. */
	firstExc = true;
	return result;
}

std::vector<const StackFrame*> Plutonium::_CrtGetStackTrace(uint32 framesToSkip)
{
	/* Reset loading exception handler and create default result. */
	firstExc = true;
	std::vector<const StackFrame*> result;

#if defined(_WIN32)
	/* Get the stack trace on windows systems. */

	/* Get current process handle. */
	HANDLE process = GetCurrentProcess();
	InitializeProcess(process);

	/* Initialize symbol info. */
	SYMBOL_INFO *infoSymbol = reinterpret_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)));
	infoSymbol->MaxNameLen = MAX_SYM_NAME;
	infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	/* Initialize file info. */
	IMAGEHLP_LINE64 *infoFile = malloc_s(IMAGEHLP_LINE64, 1);
	infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	/* Initialize the module info. */
	IMAGEHLP_MODULE64 *infoModule = malloc_s(IMAGEHLP_MODULE64, 1);
	infoModule->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

	/* We don't use displacement but the function needs it. */
	DWORD displ;

	/* Get required stack frame. */
	void *frames[32];
	uint16 frameCnt = CaptureStackBackTrace(framesToSkip, 32, frames, nullptr);

	/* Add information to all frames. */
	for (size_t i = 0; i < frameCnt; i++)
	{
		uint64 address = reinterpret_cast<uint64>(frames[i]);
		StackFrame *cur = new StackFrame();

		/* Attempt to get symbol information from address. */
		if (SymFromAddr(process, address, 0, infoSymbol)) cur->FunctionName = heapstr(infoSymbol->Name);

		/* Attempt to get file information from address. */
		if (SymGetLineFromAddr64(process, address, &displ, infoFile))
		{
			cur->FileName = heapstr(infoFile->FileName);
			cur->Line = infoFile->LineNumber;
		}

		/* Attempt to get the module information from address. */
		if (SymGetModuleInfo64(process, address, infoModule)) cur->ModuleName = heapstr(infoModule->ModuleName);

		result.push_back(cur);
	}

	free_s(infoSymbol);
	free_s(infoFile);
	free_s(infoModule);
#else
	LOG_WAR_ONCE("Cannot get stack trace on this platform!");
#endif

	/* Reset loading exception handler and return result. */
	firstExc = true;
	return result;
}

Plutonium::StackFrame::~StackFrame(void)
{
	/* 
	Because the strings within the symbols are only there we must copy them to the heap.
	Thusly if they aren't their default value, free them.
	*/
	if (strcmp(FunctionName, "Unknown")) free_s(FunctionName);
	if (strcmp(FileName, "Unknown")) free_s(FileName);
	if (strcmp(ModuleName, "Unknown")) free_s(ModuleName);
}