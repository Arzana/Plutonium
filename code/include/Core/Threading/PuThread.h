#pragma once
#include "Core/String.h"
#include <thread>

namespace Pu
{
	/* Defines a helper object for thread and process handling. */
	class PuThread
	{
	public:
		PuThread(void) = delete;
		PuThread(_In_ const PuThread&) = delete;
		PuThread(_In_ PuThread&&) = delete;

		_Check_return_ PuThread& operator =(_In_ const PuThread&) = delete;
		_Check_return_ PuThread& operator =(_In_ PuThread&&) = delete;

		/* Sets (or overrides) the name of the calling thread. */
		static void SetName(_In_ const wstring &name);
		/* Waits for the specified process to stop execution. */
		_Check_return_ static bool TryWait(_In_ uint64 process, _In_opt_ uint32 timeout = 0);
		/* Waits for the specified thread to stop execution. */
		static void Wait(_In_ std::thread &thread);
		/* Waits for all specified threads to stop execution. */
		static void WaitAll(_In_ vector<std::thread> &threads);
		/* Locks the calling thread to a specific CPU core. */
		static void Lock(_In_ uint64 core);
		/* Locks the specified thread to a specific CPU core. */
		static void Lock(_In_ std::thread &thread, _In_ uint64 core);
		/* Commands the calling thread to sleep for a specific amount of time. */
		static void Sleep(_In_ uint64 milliseconds);
		/* Hint to the processor that the calling thread is in a spin-wait loop. */
		static void Pause(void);
		/* Gets the current process ID. */
		_Check_return_ static uint64 GetProcessID(void);
		/* Gets the module name of the specified process. */
		_Check_return_ static wstring GetProcessName(_In_ uint64 process);
		/* Gets the module name of the current process. */
		_Check_return_ static wstring GetProcessName(void);
		/* Gets the name of the specified thread. */
		_Check_return_ static wstring GetThreadName(_In_ uint64 thread);
		/* Gets the name of the calling thread. */
		_Check_return_ static wstring GetThreadName(void);
		/* Gets the ID of the logical processor that the calling thread is running on. */
		_Check_return_ static uint64 GetProcessorID(void);
		/* Gets the components of a specified environment variable. */
		_Check_return_ static vector<wstring> GetEnvironmentVariable(_In_ const wstring &name);
		/* Attempts to start a child process (checks path if not found right away). */
		_Check_return_ static bool TryStartProcess(_In_ const wstring &name, _In_ const wstring &arguments, _Out_opt_ uint64 *handle, _In_opt_ bool captureOutput = false);
		/* Gets the STDOUT from the specified child process (only available if captureOutput was true). */
		_Check_return_ static string GetProcessOutput(_In_ uint64 process);

	private:
		static void LockInternal(std::thread::native_handle_type hndl, uint64 core);
	};
}