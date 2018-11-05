#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/StackTrace.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Streams/FileUtils.h"

#ifdef _WIN32
#include <Windows.h>	// GetCurrentProcessId, OpenProcess, CloseHandle, GetThreadDescription.
#include <Psapi.h>		// GetModuleBaseName
#include <comdef.h>		// wchar_t to char
#endif

using namespace Pu;

uint64 Pu::_CrtGetCurrentProcessId(void)
{
#if defined(_WIN32)
	/* On windows use windows API to get process id. */
	return static_cast<uint64>(GetCurrentProcessId());
#else
	Log::Warning("Cannot get process id on this platform!");
	return 0;
#endif
}

string Pu::_CrtGetProcessNameFromId(uint64 id)
{
#if defined(_WIN32)
	/* Attempt to open the process information. */
	const HANDLE phndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, static_cast<DWORD>(id));
	if (phndl)
	{
		/* Attempt to get module base name. */
		CHAR buffer[MAX_PATH];
		if (GetModuleBaseName(phndl, nullptr, buffer, sizeof(buffer)))
		{
			CloseHandle(phndl);
			return _CrtGetFileNameWithoutExtension(buffer);
		}
	}

	/* Could not open process or get module name; log error. */
	CloseHandle(phndl);
	const string error = _CrtGetErrorString();
	Log::Warning("Could not get process name, error: %s!", error.c_str());
	return "";

#else
	Log::Warning("Cannot get process name on this platform!");
	return "";
#endif
}

uint64 Pu::_CrtGetCurrentThreadId(void)
{
	return static_cast<uint64>(_threadid);
}

string Pu::_CrtGetThreadNameFromId(uint64 id)
{
#if defined(_WIN32)
	/* Attempt to open the thread information. */
	const HANDLE thndl = OpenThread(THREAD_QUERY_INFORMATION, false, static_cast<DWORD>(id));
	if (thndl)
	{
		/* Attempt to get thread description. */
		PWSTR desc;
		if (GetThreadDescription(thndl, &desc))
		{
			_bstr_t b(desc);
			if (b.length() > 0)
			{
				/* Convert the description to a normal char and return it. */
				CloseHandle(thndl);
				return string(b);
			}
		}
	}

	/* Could not open thread or get description; log error. */
	CloseHandle(thndl);
	const string error = _CrtGetErrorString();
	if (error.length() > 0) Log::Warning("Could not get thread name, error: %s!", error.c_str());

	/*
	Description is empty so we need to get the module name of the thread creator;
	ignore the last 2 frames because those are the ones creating the thread.
	- ...
	- <the one we want>
	- kernel32.dll
	- ntdll.dll
	*/
	StackFrame frame;
	StackFrame::GetCallerInfo(-3, frame);
	return frame.ModuleName;

#else
	Log::Error("Cannot get thread name on this platform!");
	return "";
#endif
}

void Pu::_CrtSetCurrentThreadName(const char * name)
{
#if defined(_WIN32)
	/* Attempt to open thread information. */
	const HANDLE thndl = OpenThread(THREAD_SET_LIMITED_INFORMATION, false, static_cast<DWORD>(_CrtGetCurrentThreadId()));
	if (thndl)
	{
		_bstr_t b(name);
		if (SetThreadDescription(thndl, b)) return;
	}

	/* Could not open thread or set description; log error. */
	CloseHandle(thndl);
	const string error = _CrtGetErrorString();
	Log::Warning("Could not set thread name, error: %s!", error.c_str());

#else
	Log::Error("Cannot set thread name on this platform!");
#endif
}