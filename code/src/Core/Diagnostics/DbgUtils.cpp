#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <DbgHelp.h>
#include <Psapi.h>
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

Pu::vector<Pu::wstring> Pu::_CrtGetLoadedModules(uint64 processID)
{
	vector<wstring> result;

#ifdef _WIN32
	const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, static_cast<DWORD>(processID));
	if (hProcess)
	{
		HMODULE hModules[1024];
		DWORD byteCount;

		/* Attempt to get the handles to all modules. */
		if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &byteCount))
		{
			/* Enumerate through the result. */
			for (size_t i = 0; i < byteCount / sizeof(HMODULE); i++)
			{
				/* Attempt to get the module name. */
				WCHAR name[MAX_PATH];
				if (GetModuleFileNameExW(hProcess, hModules[i], name, sizeof(name) / sizeof(WCHAR)))
				{
					result.emplace_back(name);
				}
			}
		}
		else Log::Error("Unable to enumerate process modules for process %#016x: %ls", hProcess, _CrtGetErrorString().c_str());

		CloseHandle(hProcess);
	}
	else Log::Error("Unable to open process %ull: %ls!", processID, _CrtGetErrorString().c_str());
#else
	Log::Error("Cannot get loaded modules on this platform!");
#endif

	return result;
}

#ifdef _WIN32
Pu::wstring Pu::_CrtFormatError(uint64 error)
{
	constexpr DWORD FLAGS = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	constexpr DWORD LANGID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

	/* Get human readable error from system. */
	LPWSTR msgBuffer = nullptr;
	FormatMessage(FLAGS, nullptr, static_cast<DWORD>(error), LANGID, reinterpret_cast<LPWSTR>(&msgBuffer), 0, nullptr);

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