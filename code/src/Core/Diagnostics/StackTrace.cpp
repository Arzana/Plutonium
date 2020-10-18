#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <DbgHelp.h>
#endif

#include "Core/Diagnostics/StackTrace.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Logging.h"

using namespace Pu;

uint64 Pu::StackFrame::GetCallerHndl(int32 framesToSkip)
{
	uint64 hndl;

#ifdef _WIN32
	if (_CrtInitializeWinProcess())
	{
		void *frames[1];
		const uint16 cnt = CaptureStackBackTrace(framesToSkip, 1, frames, nullptr);
		hndl = reinterpret_cast<uint64>(frames[0]);
	}
	else hndl = 0;
#else
	Log::Error("Cannot get caller frame handle on this platform!");
#endif

	return hndl;
}

bool Pu::StackFrame::GetCallerInfoFromHndl(uint64 hndl, StackFrame & frame)
{
	bool partiallyFailed = false;

#ifdef _WIN32
	/* Get current process handle. */
	const HANDLE process = GetCurrentProcess();
	if (_CrtInitializeWinProcess())
	{
		/* Initialize symbol info. */
		SYMBOL_INFOW *infoSymbol = reinterpret_cast<SYMBOL_INFOW*>(malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)));
		infoSymbol->MaxNameLen = MAX_SYM_NAME;
		infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);

		/* Initialize file info. */
		IMAGEHLP_LINEW64 *infoFile = new IMAGEHLP_LINEW64();
		infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

		/* Initialize the module info. */
		IMAGEHLP_MODULEW64 *infoModule = new IMAGEHLP_MODULEW64();
		infoModule->SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);

		/* We don't use displacement but the function needs it. */
		DWORD displ;

		/* Attempt to get symbol information from address. */
		if (SymFromAddrW(process, hndl, 0, infoSymbol)) frame.FunctionName = infoSymbol->Name;
		else
		{
			Log::Warning("Unable to load function name from symbols (%ls)!", _CrtGetErrorString().c_str());
			partiallyFailed = true;
		}

		/* Attempt to get file information from address. */
		if (SymGetLineFromAddrW64(process, hndl, &displ, infoFile))
		{
			frame.FileName = infoFile->FileName;
			frame.Line = infoFile->LineNumber;
		}
		else
		{
			Log::Warning("Unable to load file name and line number from symbols (%ls)!", _CrtGetErrorString().c_str());
			partiallyFailed = true;
		}

		/* Attempt to get the module information from address. */
		if (SymGetModuleInfoW64(process, hndl, infoModule)) frame.ModuleName = infoModule->ModuleName;
		else
		{
			Log::Warning("Unable to load module name from symbols (%ls)!", _CrtGetErrorString().c_str());
			partiallyFailed = true;
		}

		free(infoSymbol);
		delete infoFile;
		delete infoModule;
	}
	else partiallyFailed = true;
#else
	Log::Error("Cannot get caller frame information on this platform!");
#endif

	return partiallyFailed;
}

bool Pu::StackFrame::GetCallerInfo(int32 framesToSkip, StackFrame & frame)
{
	bool partiallyFailed = false;

#ifdef _WIN32
	/* Get current process handle. */
	const HANDLE process = GetCurrentProcess();
	if (_CrtInitializeWinProcess())
	{
		/* Initialize symbol info. */
		SYMBOL_INFOW *infoSymbol = reinterpret_cast<SYMBOL_INFOW*>(malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)));
		infoSymbol->MaxNameLen = MAX_SYM_NAME;
		infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);

		/* Initialize file info. */
		IMAGEHLP_LINEW64 *infoFile = new IMAGEHLP_LINEW64();
		infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

		/* Initialize the module info. */
		IMAGEHLP_MODULEW64 *infoModule = new IMAGEHLP_MODULEW64();
		infoModule->SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);

		/* We don't use displacement but the function needs it. */
		DWORD displ;

		/* Get required stack frame. */
		void *frames[32];
		uint16 frameCnt = CaptureStackBackTrace(framesToSkip > 0 ? framesToSkip : 0, 32, frames, nullptr);

		/* Loop through frames untill a valid frame is found. */
		bool hasFunc = false, hasFile = false, hasModule = false;
		for (size_t i = framesToSkip >= 0 ? 0 : frameCnt + framesToSkip; i < frameCnt && !(hasFunc || hasFile || hasModule); i++)
		{
			const uint64 address = reinterpret_cast<uint64>(frames[i]);

			/* Attempt to get symbol information from address. */
			if (!hasFunc)
			{
				if (SymFromAddrW(process, address, 0, infoSymbol))
				{
					hasFunc = true;
					frame.FunctionName = infoSymbol->Name;
				}
				else
				{
					Log::Warning("Unable to load function name from symbols (%ls)!", _CrtGetErrorString().c_str());
					partiallyFailed = true;
				}
			}

			/* Attempt to get file information from address. */
			if (!hasFile)
			{
				if (SymGetLineFromAddrW64(process, address, &displ, infoFile))
				{
					hasFile = true;
					frame.FileName = infoFile->FileName;
					frame.Line = infoFile->LineNumber;
				}
				else
				{
					Log::Warning("Unable to load file name and line number from symbols (%ls)!", _CrtGetErrorString().c_str());
					partiallyFailed = true;
				}
			}

			/* Attempt to get the module information from address. */
			if (!hasModule)
			{
				if (SymGetModuleInfoW64(process, address, infoModule))
				{
					hasModule = true;
					frame.ModuleName = infoModule->ModuleName;
				}
				else
				{
					Log::Warning("Unable to load module name from symbols (%ls)!", _CrtGetErrorString().c_str());
					partiallyFailed = true;
				}
			}
		}

		free(infoSymbol);
		delete infoFile;
		delete infoModule;
	}
	else partiallyFailed = true;
#else
	Log::Error("Cannot get caller information on this platform!");
#endif

	return partiallyFailed;
}

bool Pu::StackFrame::GetStackTrace(int32 framesToSkip, vector<StackFrame>& frames)
{
	bool partiallyFailed = false;

#if defined(_WIN32)
	/* Get the stack trace on windows systems. */

	/* Get current process handle. */
	const HANDLE process = GetCurrentProcess();
	if (_CrtInitializeWinProcess())
	{
		/* Initialize symbol info. */
		SYMBOL_INFOW *infoSymbol = reinterpret_cast<SYMBOL_INFOW*>(malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)));
		infoSymbol->MaxNameLen = MAX_SYM_NAME;
		infoSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);

		/* Initialize file info. */
		IMAGEHLP_LINEW64 *infoFile = new IMAGEHLP_LINEW64();
		infoFile->SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

		/* Initialize the module info. */
		IMAGEHLP_MODULEW64 *infoModule = new IMAGEHLP_MODULEW64();
		infoModule->SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);

		/* We don't use displacement but the function needs it. */
		DWORD displ;

		/* Get required stack frame. */
		void *rawFrames[32];
		uint16 frameCnt = CaptureStackBackTrace(framesToSkip, 32, rawFrames, nullptr);

		/* Add information to all frames. */
		for (size_t i = 0; i < frameCnt; i++)
		{
			const uint64 address = reinterpret_cast<uint64>(rawFrames[i]);
			StackFrame frame;

			/* Attempt to get symbol information from address. */
			if (SymFromAddrW(process, address, 0, infoSymbol)) frame.FunctionName = infoSymbol->Name;

			/* Attempt to get file information from address. */
			if (SymGetLineFromAddrW64(process, address, &displ, infoFile))
			{
				frame.FileName = infoFile->FileName;
				frame.Line = infoFile->LineNumber;
			}

			/* Attempt to get the module information from address. */
			if (SymGetModuleInfoW64(process, address, infoModule)) frame.ModuleName = infoModule->ModuleName;

			frames.push_back(frame);
		}

		free(infoSymbol);
		delete infoFile;
		delete infoModule;
	}
	else partiallyFailed = true;
#else
	Log::Error("Cannot get stack trace on this platform!");
#endif

	return partiallyFailed;
}