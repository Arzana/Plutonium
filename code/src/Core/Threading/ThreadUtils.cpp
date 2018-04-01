#include "Core\Threading\ThreadUtils.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Streams\FileReader.h"

#if defined(_WIN32)
#include <Windows.h>	// GetCurrentProcessId, OpenProcess, CloseHandle, GetThreadDescription.
#include <Psapi.h>		// GetModuleBaseName
#include <comdef.h>		// wchar_t to char
#endif

using namespace Plutonium;

uint64 Plutonium::_CrtGetCurrentProcessId(void)
{
#if defined(_WIN32)
	/* On windows use windows API to get process id. */
	return static_cast<uint64>(GetCurrentProcessId());
#else
	LOG_WAR_ONCE("Cannot get process id on this platform!");
	return 0;
#endif
}

const char * Plutonium::_CrtGetProcessNameFromId(uint64 id)
{
#if defined(_WIN32)
	/* Attempt to open the process information. */
	HANDLE phndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, static_cast<DWORD>(id));
	if (phndl)
	{
		/* Attempt to get module base name. */
		CHAR buffer[MAX_PATH];
		if (GetModuleBaseName(phndl, nullptr, buffer, sizeof(buffer)))
		{
			CloseHandle(phndl);
			FileReader fr(buffer, true);
			return heapstr(fr.GetFileNameWithoutExtension());
		}
	}

	/* Could not open process or get module name; log error. */
	CloseHandle(phndl);
	const char *error = _CrtGetErrorString();
	LOG_WAR("Could not get process name, error: %s!", error);
	free_cstr_s(error);
	return "";

#else
	LOG_WAR_ONCE("Cannot get process name on this platform!");
	return "";
#endif
}

uint64 Plutonium::_CrtGetCurrentThreadId(void)
{
	return static_cast<uint64>(_threadid);
}

const char * Plutonium::_CrtGetThreadNameFromId(uint64 id)
{
#if defined(_WIN32)
	/* Attempt to open the thread information. */
	HANDLE thndl = OpenThread(THREAD_QUERY_INFORMATION, false, static_cast<DWORD>(id));
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
				return heapstr(b);
			}
			else
			{
				/* Description is empty so we need to get the module name of the thread creator; ignore the last 2 frames because those are the ones creating the thread. */
				const StackFrame *frame = _CrtGetCallerInfo(-3);
				const char *result = heapstr(frame->ModuleName);
				delete_s(frame);
				return result;
			}
		}
	}

	/* Could not open thread or get description; log error. */
	CloseHandle(thndl);
	const char *error = _CrtGetErrorString();
	LOG_WAR("Could not get thread name, error: %s!", error);
	free_cstr_s(error);
	return "";

#else
	LOG_WAR_ONCE("Cannot get thread name on this platform!");
	return "";
#endif
}

void Plutonium::_CrtSetCurrentThreadName(const char * name)
{
#if defined(_WIN32)
	/* Attempt to open thread information. */
	HANDLE thndl = OpenThread(THREAD_SET_LIMITED_INFORMATION, false, static_cast<DWORD>(_CrtGetCurrentThreadId()));
	if (thndl)
	{
		_bstr_t b(name);
		if (SetThreadDescription(thndl, b)) return;
	}

	/* Could not open thread or set description; log error. */
	CloseHandle(thndl);
	const char *error = _CrtGetErrorString();
	LOG_WAR("Could not set thread name, error: %s!", error);
	free_cstr_s(error);

#else
	LOG_WAR_ONCE("Cannot set thread name on this platform!");
#endif
}
