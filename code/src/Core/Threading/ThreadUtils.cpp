#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"	// GetCurrentProcessId, OpenProcess, CloseHandle, GetThreadDescription.
#include <Psapi.h>							// GetModuleBaseName
#include <comdef.h>							// wchar_t to char
#endif

#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/StackTrace.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Logging.h"

using namespace Pu;

template <typename _Ty>
static inline _Ty CreateZeroStruct()
{
	_Ty result;
	ZeroMemory(&result, sizeof(_Ty));
	return result;
}

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

wstring Pu::_CrtGetProcessNameFromId(uint64 id)
{
#if defined(_WIN32)
	/* Attempt to open the process information. */
	const HANDLE phndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, static_cast<DWORD>(id));
	if (phndl)
	{
		/* Attempt to get module base name. */
		WCHAR buffer[MAX_PATH];
		if (GetModuleBaseName(phndl, nullptr, buffer, sizeof(buffer)))
		{
			CloseHandle(phndl);
			return wstring(buffer).fileNameWithoutExtension();
		}
	}

	/* Could not open process or get module name; log error. */
	CloseHandle(phndl);
	Log::Warning("Could not get process name, error: %ls!", _CrtGetErrorString().c_str());
	return L"";

#else
	Log::Warning("Cannot get process name on this platform!");
	return L"";
#endif
}

uint64 Pu::_CrtGetCurrentThreadId(void)
{
	return static_cast<uint64>(_threadid);
}

wstring Pu::_CrtGetThreadNameFromId(uint64 id)
{
#if defined(_WIN32)
	/* Attempt to open the thread information. */
	const HANDLE thndl = OpenThread(THREAD_QUERY_INFORMATION, false, static_cast<DWORD>(id));
	if (thndl)
	{
		/* Attempt to get thread description. */
		PWSTR desc;
		if (GetThreadDescription(thndl, &desc) >= 0)
		{
			/* If no description is set we just return nothing. */
			_bstr_t b(desc);
			if (b.length() > 0)
			{
				/* Convert the description to a normal wchar and return it. */
				CloseHandle(thndl);
				return wstring(b);
			}
		}
		else Log::Warning("Could not get thread name '%ls'!", _CrtGetErrorString().c_str());
	}

	/* Could not open thread or get description; log error. */
	if (!CloseHandle(thndl)) Log::Warning("Unable to close thread query information handle '%ls'!", _CrtGetErrorString().c_str());

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
	return L"";
#endif
}

void Pu::_CrtSetCurrentThreadName(const wstring & name)
{
#if defined(_WIN32)
	/* Attempt to open thread information. */
	const HANDLE thndl = OpenThread(THREAD_SET_LIMITED_INFORMATION, false, static_cast<DWORD>(_CrtGetCurrentThreadId()));
	if (thndl)
	{
		_bstr_t b(name.c_str());
		if (SetThreadDescription(thndl, b)) return;
	}

	/* Could not open thread or set description; log error. */
	CloseHandle(thndl);
	Log::Warning("Could not set thread name, error: %ls!", _CrtGetErrorString().c_str());

#else
	Log::Error("Cannot set thread name on this platform!");
#endif
}

vector<wstring> Pu::_CrtGetEnviromentVariables(const wstring & name)
{
	/* Gets the raw enviroment variables. */
	wchar_t *raw;
	size_t len;
	const errno_t result = _wdupenv_s(&raw, &len, name.c_str());

	/* Check for error. */
	if (result != NO_ERROR)
	{
		Log::Error("Unable to get enviroment variables (%ls)!", _CrtFormatError(result).c_str());
		free(raw);
		return vector<wstring>();
	}
	else
	{
		/* Free the raw string and return the split varient of the variables for ease of use. */
		const wstring str(raw);
		free(raw);
		return str.split(L';');
	}
}

bool Pu::_CrtRunProcess(const wstring & name, wstring & arguments, wstring & output, uint64 timeout)
{
	/* Get OS path variables. */
	const vector<wstring> pathDirs = _CrtGetEnviromentVariables(L"Path");
	const size_t maxIdx = pathDirs.size() - 1;
	wstring dir = L"";
	size_t nextDirIdx = 0;

#ifdef _WIN32
	/* Create security object used to redirect STDOUT. */
	SECURITY_ATTRIBUTES sAttr = CreateZeroStruct<SECURITY_ATTRIBUTES>();
	sAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	sAttr.bInheritHandle = true;

	/* Create pipe for child process's STDOUT. */
	HANDLE childStdOutRead = nullptr, childStdOutWrite = nullptr;
	if (!CreatePipe(&childStdOutRead, &childStdOutWrite, &sAttr, 0))
	{
		Log::Warning("Unable to create read and write pipe for child process STDOUT (%ls)!", _CrtGetErrorString().c_str());
	}

	/* Ensure that the read handle to the pipe for STDOUT is not inherited. */
	if (!SetHandleInformation(childStdOutRead, HANDLE_FLAG_INHERIT, 0))
	{
		Log::Warning("Unable to ensure STDOUT is not inherited (%ls)!", _CrtGetErrorString().c_str());
	}

	/* Specify that the child should use the specified STDOUT handles. */
	STARTUPINFO sInfo = CreateZeroStruct<STARTUPINFO>();
	sInfo.cb = sizeof(STARTUPINFO);
	sInfo.hStdError = childStdOutWrite;
	sInfo.hStdOutput = childStdOutWrite;
	if (childStdOutWrite) sInfo.dwFlags |= STARTF_USESTDHANDLES;

	/* Create process information object. */
	PROCESS_INFORMATION pInfo = CreateZeroStruct<PROCESS_INFORMATION>();

	/* Try to start the process, first from it's name and then for each directory defined in system path. */
	bool started = false;
	do
	{
		/* Create the final process name and add it to the argument list as the first argument (C expects this). */
		const wstring pname = dir.length() > 0 ? dir + L'\\' + name : name;
		wstring argv = pname + L' ' + arguments;

		started = CreateProcess(
			pname.c_str(),
			const_cast<wchar_t*>(argv.c_str()),
			nullptr,
			nullptr,
			childStdOutWrite != nullptr,
			0,
			nullptr,
			nullptr,
			&sInfo, &pInfo);

		if (nextDirIdx < maxIdx)
		{
			dir = pathDirs.at(nextDirIdx);
			++nextDirIdx;
		}
	} while (!started && nextDirIdx < maxIdx);

	/* If the process was successfully started, let it run and delete it's handle after the timeout. */
	if (started)
	{
		/* Wait for the process to exit or until the timeout has passed.. */
		const DWORD waitResult = WaitForSingleObject(pInfo.hProcess, static_cast<DWORD>(timeout));
		if (waitResult == WAIT_OBJECT_0)
		{
			/* Get the STDOUT buffer size. */
			DWORD requiredSize, read = 0;
			if (PeekNamedPipe(childStdOutRead, nullptr, 0, nullptr, &requiredSize, nullptr))
			{
				/* Reserve space for log and read the log. */
				output.resize(requiredSize);
				if (!ReadFile(childStdOutRead, output.data(), requiredSize, &read, nullptr))
				{
					Log::Warning("Unable to read from STDOUT pipe (%ls)!", _CrtGetErrorString().c_str());
				}
			}
			else Log::Warning("Unable to request STDOUT pipe size (%ls)!", _CrtGetErrorString().c_str());
		}
		else if (waitResult == WAIT_TIMEOUT) Log::Warning("Child process '%s' has timed out, releasing handle!", name.c_str());
		else if (waitResult == WAIT_FAILED)	Log::Error("Child process '%s' has crashed (%ls)!", name.c_str(), _CrtGetErrorString().c_str());

		/* Close handles to the child. */
		CloseHandle(pInfo.hProcess);
		CloseHandle(pInfo.hThread);
		return waitResult == WAIT_OBJECT_0;
	}

	/* Process creation failed so log error and return false. */
	Log::Error("Unable to start process '%s' (%ls)!", name.c_str(), _CrtGetErrorString().c_str());
	return false;
#else
	Log::Warning("Creating child processes is not supported on this platform!");
	return false;
#endif
}