#include "Core/Threading/PuThread.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/CPU.h"
#include "Core/Diagnostics/StackTrace.h"

#ifdef _WIN32
#include <Psapi.h>
#include "Core/Platform/Windows/Windows.h"

/* Key: Process, Value: [Thread, Read, Write] */
std::map<HANDLE, std::tuple<HANDLE, HANDLE, HANDLE>> win32ProcessHandles;

bool tryStartProcessWin32(const Pu::wstring &dir, const Pu::wstring &name, const Pu::wstring &arguments, bool inheritHandles, LPSTARTUPINFO sInfo, LPPROCESS_INFORMATION pInfo)
{
	const Pu::wstring pName = dir.length() > 0 ? dir + L"\\" + name : name;
	Pu::wstring argv = pName + L' ' + arguments;
	return CreateProcess(pName.c_str(), argv.data(), nullptr, nullptr, inheritHandles, 0, nullptr, nullptr, sInfo, pInfo);
}

bool addProcessWin32(LPPROCESS_INFORMATION pInfo, HANDLE read, HANDLE write, Pu::uint64 *handle)
{
	win32ProcessHandles.emplace(pInfo->hProcess, std::make_tuple(pInfo->hThread, read, write));
	if (handle) *handle = reinterpret_cast<Pu::uint64>(pInfo->hProcess);
	return true;
}

void destroyProcessWin32(HANDLE key)
{
	const auto[hThread, hRead, hWrite] = win32ProcessHandles.at(key);
	win32ProcessHandles.erase(key);

	CloseHandle(key);
	CloseHandle(hThread);
	CloseHandle(hRead);
	CloseHandle(hWrite);
}
#endif

void Pu::PuThread::SetName(const wstring & name)
{
#ifdef _WIN32
	/* We only need limited information access in order to set the thread name. */
	const HANDLE hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, false, _threadid);
	if (hThread)
	{
		if (!SetThreadDescription(hThread, name.c_str())) Log::Error("Unable to set thread name (%ls)!", _CrtGetErrorString().c_str());
		CloseHandle(hThread);
	}
	else Log::Error("Unable to set thread name, could not open thread (%ls)!", _CrtGetErrorString().c_str());
#else
	Log::Warning("Unable to set thread name on this platform!");
#endif
}

bool Pu::PuThread::TryWait(uint64 process, uint32 timeout)
{
#ifdef _WIN32
	/* The only successful wait is one that results in WAIT_OBJECT_0. */
	const HANDLE hProcess = reinterpret_cast<HANDLE>(process);
	const DWORD result = WaitForSingleObject(hProcess, timeout);
	if (result == WAIT_FAILED)
	{
		/* The child thread has failed execution, but that also means it's done. */
		Log::Error("Process 0x%X failed (%ls)!", _CrtGetErrorString().c_str());
	}

	/* Make sure to destroy the handle if output read was not requested. */
	if (result == WAIT_OBJECT_0 || result == WAIT_FAILED)
	{
		decltype(win32ProcessHandles)::const_iterator it = win32ProcessHandles.find(hProcess);
		if (it != win32ProcessHandles.end() && !std::get<1>(it->second)) destroyProcessWin32(hProcess);
		return true;
	}
#else
	Log::Warning("Unable to wait for process on this platform!");
#endif

	/* Either the timeout was reached or a spin lock in required. */
	return false;
}

void Pu::PuThread::Wait(std::thread & thread)
{
	if (thread.joinable()) thread.join();
}

void Pu::PuThread::WaitAll(vector<std::thread>& threads)
{
	for (std::thread &thread : threads) PuThread::Wait(thread);
}

void Pu::PuThread::Lock(uint64 core)
{
#ifdef _WIN32
	LockInternal(GetCurrentThread(), core);
#else
	Log::Warning("Unable to lock current thread on this platform!");
#endif
}

void Pu::PuThread::Lock(std::thread & thread, uint64 core)
{
	LockInternal(thread.native_handle(), core);
}

void Pu::PuThread::Sleep(uint64 milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Pu::PuThread::Pause(void)
{
	/* 
	The pause instruction actually signals to the CPU that we're in a spin lock,
	but it's not guaranteed to be supported, so sleep for 1 millisecond if it isn't.
	*/
	if (CPU::SupportsSSE2()) _mm_pause();
	else Sleep(1);
}

Pu::uint64 Pu::PuThread::GetProcessID(void)
{
#ifdef _WIN32
	return static_cast<uint64>(GetCurrentProcessId());
#else
	Log::Warning("Cannot get process ID on this platform!");
	return 0;
#endif
}

Pu::wstring Pu::PuThread::GetProcessName(uint64 process)
{
#ifdef _WIN32
	/* Attempt to open the process information. */
	const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, static_cast<DWORD>(process));
	if (hProcess)
	{
		/* Attempt to get the module base name. */
		wstring result(static_cast<size_t>(MAX_PATH));
		if (GetModuleBaseName(hProcess, nullptr, result.data(), MAX_PATH))
		{
			CloseHandle(hProcess);
			return result.fileNameWithoutExtension();
		}

		CloseHandle(hProcess);
	}

	/* Either could not open the process or get the module name. */
	Log::Warning("Unable to get module name for process 0x%X (%ls)!", process, _CrtGetErrorString().c_str());
	return L"";
#else
	Log::Warning("Cannot get process name on this platform!");
	return L"";
#endif
}

Pu::wstring Pu::PuThread::GetProcessName(void)
{
	return GetProcessName(GetProcessID());
}

Pu::wstring Pu::PuThread::GetThreadName(uint64 thread)
{
#ifdef _WIN32
	/* We only need limited information access in order to get the thread name. */
	const HANDLE hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, false, static_cast<DWORD>(thread));
	if (hThread)
	{
		PWSTR desc;
		if (GetThreadDescription(hThread, &desc) >= 0)
		{
			const wstring result(desc);
			LocalFree(desc);
			return result;
		}
		else Log::Error("Unable to get thread name (%ls)!", _CrtGetErrorString().c_str());

		CloseHandle(hThread);
	}
	else Log::Error("Unable to get thread name, could not open thread (%ls)!", _CrtGetErrorString().c_str());
#endif

	/*
	Description is empty so we need to get the module name of the thread creator;
	ignore the last 2 frames because those are the ones creating the thread.
	This is only a backup.
	- ...
	- <the one we want>
	- kernel32.dll
	- ntdll.dll
	*/
	StackFrame frame;
	StackFrame::GetCallerInfo(-3, frame);
	return frame.ModuleName;
}

Pu::wstring Pu::PuThread::GetThreadName(void)
{
	return GetThreadName(_threadid);
}

Pu::uint64 Pu::PuThread::GetProcessorID(void)
{
#ifdef _WIN32
	return GetCurrentProcessorNumber();
#else
	Log::Error("Cannot get processor ID on this platform!");
	return 0;
#endif
}

Pu::vector<Pu::wstring> Pu::PuThread::GetEnvironmentVariable(const Pu::wstring & name)
{
	/* Gets the raw enviroment variables. */
	wchar_t *raw;
	size_t len;
	const errno_t result = _wdupenv_s(&raw, &len, name.c_str());

	/* Check for error. */
	if (result != NO_ERROR)
	{
		Log::Error("Unable to get value for environment variable '%ls' (%ls)!", name.c_str(), _CrtFormatError(result).c_str());
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

bool Pu::PuThread::TryStartProcess(const wstring & name, const wstring & arguments, uint64 * handle, bool captureOutput)
{
#ifdef _WIN32
	HANDLE childStdOutRead = nullptr, childStdOutWrite = nullptr;

	STARTUPINFO sInfo;
	memset(&sInfo, 0, sizeof(STARTUPINFO));
	sInfo.cb = sizeof(STARTUPINFO);

	if (captureOutput)
	{
		/* Create a new pipe for the std handles. */
		SECURITY_ATTRIBUTES sAttr = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };
		if (CreatePipe(&childStdOutRead, &childStdOutWrite, &sAttr, 0))
		{
			if (SetHandleInformation(childStdOutRead, HANDLE_FLAG_INHERIT, 0))
			{
				/* Add the write handle to the start information struct. */
				sInfo.hStdError = childStdOutWrite;
				sInfo.hStdOutput = childStdOutWrite;
				sInfo.dwFlags = STARTF_USESTDHANDLES;
			}
			else Log::Warning("Unable to capture output for child process '%ls', unable to ensure STDOUT will be inherited (%ls)!", name.c_str(), _CrtGetErrorString().c_str());
		}
		else Log::Warning("Unable to capture output for child process '%ls', unable to create read pipe for STDOUT (%ls)!", name.c_str(), _CrtGetErrorString().c_str());
	}

	PROCESS_INFORMATION pInfo;
	memset(&pInfo, 0, sizeof(PROCESS_INFORMATION));

	/* Attempt to start the process in the current directory. */
	if (tryStartProcessWin32(L"", name, arguments, captureOutput, &sInfo, &pInfo)) return addProcessWin32(&pInfo, childStdOutRead, childStdOutWrite, handle);

	/* The process could not be started from the working directory, try with the path added. */
	for (const wstring &dir : PuThread::GetEnvironmentVariable(L"Path"))
	{
		if (tryStartProcessWin32(dir, name, arguments, captureOutput, &sInfo, &pInfo)) return addProcessWin32(&pInfo, childStdOutRead, childStdOutWrite, handle);
	}

	Log::Error("Unable to start process '%ls' (%ls)!", name.c_str(), _CrtGetErrorString().c_str());
#else
	Log::Warning("Unable to start child process '%ls' on this platform!", name.c_str());
#endif

	return false;
}

Pu::string Pu::PuThread::GetProcessOutput(uint64 process)
{
#ifdef _WIN32
	/* Make sure this is a know process. */
	const HANDLE hProcess = reinterpret_cast<HANDLE>(process);
	decltype(win32ProcessHandles)::const_iterator it = win32ProcessHandles.find(hProcess);
	if (it == win32ProcessHandles.end())
	{
		Log::Error("Unable to get output for process 0x%X (unknown process or secondary call)!", process);
		return "";
	}

	/* Check for invalid calls. */
	if (!std::get<1>(it->second)) Log::Fatal("Unable to get ouput for process 0x%X (process was not created with capture output enabled)!");

	DWORD size;
	if (PeekNamedPipe(std::get<1>(it->second), nullptr, 0, nullptr, &size, nullptr))
	{
		string output(static_cast<size_t>(size));
		DWORD read;

		if (ReadFile(std::get<1>(it->second), output.data(), size, &read, nullptr)) return output;
		Log::Error("Unable to get output for process 0x%X, unable to read from STDOUT pipe (%ls)!", process, _CrtGetErrorString().c_str());
	}
	else Log::Error("Unable to get output for process 0x%X, unable to get STDOUT size (%ls)!", process, _CrtGetErrorString().c_str());

	/* Make sure to close the handles to the pipes and process. */
	destroyProcessWin32(hProcess);
#else
	Log::Warning("Cannot get process output on this platform!");
	return "";
#endif
}

void Pu::PuThread::LockInternal(std::thread::native_handle_type hndl, uint64 core)
{
#ifdef _WIN32
	if (!SetThreadAffinityMask(hndl, 1ull << core))
	{
		Log::Error("Failed to lock thread to CPU core %zu (%ls)!", core, _CrtGetErrorString().c_str());
	}
#else
	Log::Warning("Unable to lock thread to specific CPU core on this platform!");
#endif
}