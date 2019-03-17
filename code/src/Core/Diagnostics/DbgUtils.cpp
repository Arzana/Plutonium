#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <DbgHelp.h>
#endif

#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Collections/Vector.h"

#ifdef _WIN32
#define WinSymInitialize				ASCII_UNICODE(SymInitialize, SymInitializeW)

Pu::vector<HANDLE> initializedProcesses;
#endif

Pu::wstring Pu::_CrtGetErrorString(void)
{
#ifdef _WIN32
	const DWORD errCode = GetLastError();
	if (errCode == NO_ERROR) return L"No Error";

	return _CrtFormatError(errCode);
#else
	Log::Error("Cannot get error string on this platform!");
	return U"";
#endif
}

#ifdef _WIN32
Pu::wstring Pu::_CrtFormatError(uint64 error)
{
	/* Get human readable error from system. */
	LPWSTR msgBuffer = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, static_cast<DWORD>(error), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuffer, 0, nullptr);

	/* Remove newline characters from error. */
	wstring result = msgBuffer;
	result.remove({ L'\n', L'\r' });

	/* Free the temporary buffer and return the result. */
	LocalFree(msgBuffer);
	return result;
}

bool Pu::_CrtInitializeWinProcess(void)
{
	/* Getting debug symbols is only supported on debug mode. */
#ifdef _DEBUG
	/* Get the current process handle. */
	const HANDLE process = GetCurrentProcess();

	/* Check if it's already initialized. */
	for (const HANDLE cur : initializedProcesses)
	{
		if (process == cur) return true;
	}

	/* Attempt to initialize process. */
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	if (!WinSymInitialize(process, nullptr, true))
	{
		Log::Error("Unable to initialize process %#016x: %ls", process, _CrtGetErrorString().c_str());
		return false;
	}
	
	/* Push it to the loaded list and return. */
	initializedProcesses.emplace_back(process);
	Log::Message("Initialized debug symbols for process %#016x.", process);
	return true;
#else
	return false;
#endif
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