#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <DbgHelp.h>
#endif

#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Collections/Vector.h"

#ifdef _WIN32
Pu::vector<HANDLE> initializedProcesses;
#endif

Pu::string Pu::_CrtGetErrorString(void)
{
#ifdef _WIN32
	const DWORD errCode = GetLastError();
	if (errCode == NO_ERROR) return "No Error";

	return _CrtFormatError(errCode);
#else
	Log::Error("Cannot get error string on this platform!");
	return "";
#endif
}

#ifdef _WIN32
Pu::string Pu::_CrtFormatError(uint64 error)
{
	/* Get human readable error from system. */
	LPSTR msgBuffer = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, static_cast<DWORD>(error), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, nullptr);

	/* Remove newline characters from error. */
	string result = msgBuffer;
	result.remove({ '\n', '\r' });

	/* Free the temporary buffer and return the result. */
	LocalFree(msgBuffer);
	return result;
}

bool Pu::_CrtInitializeWinProcess(void)
{
	/* Get the current process handle. */
	const HANDLE process = GetCurrentProcess();

	/* Check if it's already initialized. */
	for (size_t i = 0; i < initializedProcesses.size(); i++)
	{
		if (initializedProcesses[i] == process) return true;
	}

	/* Attempt to initialize process. */
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	if (!SymInitialize(process, nullptr, true))
	{
		const string error = _CrtGetErrorString();
		Log::Error("Unable to initialize process %#016x: %s", process, error.c_str());
		return false;
	}
	
	/* Push it to the loaded list and return. */
	initializedProcesses.push_back(process);
	Log::Verbose("Initialized debug symbols for process %#016x.", process);
	return true;
}

void Pu::_CrtFinalizeWinProcess(void)
{
	Log::Verbose("Finalizing debug symbols for %zu process(es).", initializedProcesses.size());

	for (size_t i = 0; i < initializedProcesses.size(); i++)
	{
		SymCleanup(initializedProcesses[i]);
	}
	initializedProcesses.clear();
}
#endif

#ifdef _DEBUG
void Pu::_CrtMoveDebugTerminal(const NativeWindow & wnd)
{
	const Offset2D wndPos = wnd.GetClientBounds().GetPosition();
	for (const Display &cur : Display::GetAll())
	{
		const Rect2D vp = cur.GetClientBounds();

		if (!vp.Contains(wndPos))
		{
			Log::Move(vp.Offset.X, vp.Offset.Y);
			Log::Resize(vp.Extent.Width, vp.Extent.Height);
			return;
		}
	}
}
#endif