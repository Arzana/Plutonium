#ifdef _WIN32
#include "Graphics/Platform/Windows/Win32Window.h"
#include "Graphics/Vulkan/Surface.h"
#include "Core/Diagnostics/DbgUtils.h"
#include <imgui/include/imgui.h>
#include <imgui/include/imgui_impl_win32.h>

#define WS_WINDOWED		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
#define WS_BORDERLESS	WS_OVERLAPPED | WS_MAXIMIZE

static Pu::vector<Pu::Win32Window*> activeWindows;

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static constexpr int GetLowWord(LPARAM lParam)
{
	return static_cast<int>(static_cast<short>(LOWORD(lParam)));
}

static constexpr int GetHighWord(LPARAM lParam)
{
	return static_cast<int>(static_cast<short>(HIWORD(lParam)));
}

/* Disable input event and char input logging. */
Pu::Win32Window::Win32Window(VulkanInstance & vulkan, const wstring & title)
	: NativeWindow(), title(title), shouldClose(false), AllowAltF4(true), focused(false), 
	OnInputEvent("Win32WindowOnInputEvent", true), InputDeviceAdded("Win32WindowInputDeviceAdded"), 
	InputDeviceRemoved("Win32WindowInputDeviceRemoved"), OnCharInput("Win32WindowOnCharInput", true),
	rawInputSize(sizeof(RAWINPUT))
{
	/* Push this window as an active window. */
	activeWindows.emplace_back(this);

	/* Allocate the first input buffer. */
	input = reinterpret_cast<PRAWINPUT>(calloc(1, sizeof(RAWINPUT)));

	/* Create new module handle for this module. */
	instance = GetModuleHandle(nullptr);

	/* Create window creation information. */
	WNDCLASSEX wndEx =
	{
		sizeof(WNDCLASSEX),					// size
		CS_DBLCLKS,							// style
		Win32Window::WndProc,				// proc
		0,									// cls extra
		0,									// wnd extra
		instance,							// instance
		nullptr,							// icon
		LoadCursor(nullptr, IDC_ARROW),		// cursor
		(HBRUSH)(COLOR_WINDOW + 1),			// background 
		nullptr,							// menu name
		title.c_str(),						// class name
		nullptr								// icon handle
	};

	/* Register new window class. */
	if (!RegisterClassEx(&wndEx)) Log::Fatal("Unable to register Win32 window (%ls)!", _CrtGetErrorString().c_str());

	/*
	Create new window, we can only start with a windowed window as all other windows call messages that we can't handle set i.e. resize, move.
	*/
	hndl = CreateWindow(title.c_str(), title.c_str(), WS_WINDOWED, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, instance, nullptr);
	if (!hndl) Log::Fatal("Unable to create Win32 window (%ls)!", _CrtGetErrorString().c_str());

	if constexpr (ImGuiAvailable)
	{
		/* Setup ImGui for this Win32 window. */
		if (!ImGui_ImplWin32_Init(hndl)) Log::Fatal("Unable to setup ImGui for Win32!");
	}

	/* Create new surface. */
	surface = new Surface(vulkan, instance, hndl);
}

Pu::Win32Window::~Win32Window(void)
{
	if constexpr (ImGuiAvailable)
	{
		/* Finalize ImGui for this Win32 window. */
		ImGui_ImplWin32_Shutdown();
	}

	free(input);

	/* Destroy handles. */
	delete surface;
	if (hndl) DestroyWindow(hndl);
	if (instance) UnregisterClass(title.c_str(), instance);

	/* Remove this window from the active windows. */
	activeWindows.remove(this);
}

bool Pu::Win32Window::operator==(const NativeWindow & other)
{
	try
	{
		const Win32Window &otherWin32 = dynamic_cast<const Win32Window&>(other);
		return otherWin32.hndl == hndl;
	}
	catch (std::bad_cast)
	{
		return false;
	}
}

bool Pu::Win32Window::operator!=(const NativeWindow & other)
{
	try
	{
		const Win32Window &otherWin32 = dynamic_cast<const Win32Window&>(other);
		return otherWin32.hndl != hndl;
	}
	catch (std::bad_cast)
	{
		return true;
	}
}

Pu::int32 Pu::Win32Window::GetDefaultTitleBarHeight(void)
{
	/* Thickness of the sizing border around a resizable window. */
	const int32 sizingBorder = GetSystemMetrics(SM_CYFRAME);
	/* The height of the caption area. */
	const int32 captionHeight = GetSystemMetrics(SM_CYCAPTION);
	/* The amount of border padding for a captioned window. */
	const int32 borderPadding = GetSystemMetrics(SM_CXPADDEDBORDER);

	return sizingBorder + captionHeight + borderPadding;
}

void Pu::Win32Window::Show(void)
{
	ShowWindow(hndl, mode == WindowMode::Windowed ? SW_SHOWNORMAL : SW_SHOWMAXIMIZED);
	SetFocus(hndl);
}

void Pu::Win32Window::Hide(void)
{
	ShowWindow(hndl, SW_HIDE);
}

void Pu::Win32Window::Close(void)
{
	shouldClose = true;
}

void Pu::Win32Window::Resize(Vector2 newSize)
{
	vp.Width = newSize.X;
	vp.Height = newSize.Y;
	UpdateClientArea();
}

void Pu::Win32Window::Move(Vector2 newLocation)
{
	pos = newLocation;
	UpdateClientArea();
}

void Pu::Win32Window::SetMode(WindowMode newMode)
{
	mode = newMode;

	/* Set the new mode. */
	if (mode == WindowMode::Windowed) SetWindowLong(hndl, GWL_STYLE, WS_WINDOWED);
	else if (mode == WindowMode::Borderless)
	{
		/* Borderless just means a window that has not border style and has the same dimensions as the display. */
		SetWindowLong(hndl, GWL_STYLE, WS_BORDERLESS);
		ShowWindow(hndl, SW_SHOWMAXIMIZED);
	}
	else if (mode == WindowMode::Fullscreen) Log::Warning("Full-screen mode is handled on Swapchain level not NativeWindow level!");
	else Log::Warning("Unknown Window mode passed to NativeWindow SetMode!");
}

bool Pu::Win32Window::Update(void)
{
	/* Query all messages. */
	MSG message;
	while (PeekMessage(&message, hndl, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	/* Return whether the window should remain updated. */
	return !shouldClose;
}

LRESULT Pu::Win32Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	for (Win32Window *window : activeWindows)
	{
		if (window->hndl == hWnd)
		{
			if constexpr (ImGuiAvailable) ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
			return window->HandleProc(message, wParam, lParam);
		}
	}

	/*
	Only log an unknow warning if it's not one of the specified messages.
	These messages occur before the window handle is returned and thusly we cannot match it to a window yet.
	- WM_CREATE:		Sent to incidate a new window to be created by CreateWindowEx or CreateWindow.
	- WM_GETMINMAXINFO:	Sent when the size or position of a window is about to change.
	- WM_NCCREATE:		Sent prior to the WM_CREATE message.
	- WM_NCCALCSIZE:	Sent when the size and position of the client area must be calculated.
	*/
	if (message != WM_CREATE && message != WM_GETMINMAXINFO && message != WM_NCCREATE && message != WM_NCCALCSIZE)
	{
		Log::Warning("Unknown window received at WndProc!");
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Pu::Win32Window::Move(int x, int y)
{
	ValueChangedEventArgs args(pos, Vector2(static_cast<float>(x), static_cast<float>(y)));
	if (args.OldValue != args.NewValue)
	{
		pos = args.NewValue;
		OnLocationChanged.Post(*this, args);
	}
}

void Pu::Win32Window::Resize(int w, int h)
{
	if (w > 0 && h > 0)
	{
		ValueChangedEventArgs args(Vector2(vp.Width, vp.Height), Vector2(static_cast<float>(w), static_cast<float>(h)));
		if (args.OldValue != args.NewValue)
		{
			vp.Width = args.NewValue.X;
			vp.Height = args.NewValue.Y;
			OnSizeChanged.Post(*this, args);
		}
	}
}

void Pu::Win32Window::UpdateClientArea(void)
{
	if (!SetWindowPos(hndl, HWND_TOPMOST, ipart(pos.X), ipart(pos.Y), ipart(vp.Width), ipart(vp.Height), 0))
	{
		Log::Warning("Win32 window client area update failed, reason: '%ls'!", _CrtGetErrorString().c_str());
	}
}

LRESULT Pu::Win32Window::HandleProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	bool oldFocus = focused;

	switch (message)
	{
	case (WM_CLOSE):
		shouldClose = true;
		return 0;
	case (WM_MOVE):
		Move(GetLowWord(lParam), GetHighWord(lParam));
		return 0;
	case (WM_SIZE):
		Resize(GetLowWord(lParam), GetHighWord(lParam));
		return 0;
	case (WM_SETFOCUS):
		if (!focused)
		{
			focused = true;
			OnGainedFocus.Post(*this);
		}
		return 0;
	case (WM_KILLFOCUS):
		if (!focused)
		{
			focused = false;
			OnLostFocus.Post(*this);
		}
		return 0;
	case (WM_ACTIVATE):
		focused = GetLowWord(wParam) != WA_INACTIVE;
		if (focused != oldFocus)
		{
			if (focused) OnGainedFocus.Post(*this);
			else OnLostFocus.Post(*this);
		}
		return 0;
	case (WM_SYSCOMMAND):
		return HandleSysCmd(wParam, lParam);
	case (WM_INPUT):
		return HandleInput(wParam, lParam);
	case (WM_INPUT_DEVICE_CHANGE):
		if (wParam == GIDC_ARRIVAL) InputDeviceAdded.Post(*this, (HANDLE)lParam);
		else if (wParam == GIDC_REMOVAL) InputDeviceRemoved.Post(*this, (HANDLE)lParam);
		return 0;
	case (WM_CHAR):
		OnCharInput.Post(*this, static_cast<wchar_t>(wParam));
		return 0;
	}

	return DefWindowProc(hndl, message, wParam, lParam);
}

LRESULT Pu::Win32Window::HandleSysCmd(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case (SC_KEYMENU):
		/* Disable the system menu from being toggled by keyboard input. */
		return 0;
	case (SC_CLOSE):
		/* Deny Alt-F4 request if enabled. */
		if (AllowAltF4) shouldClose = true;
		return 0;
	case (SC_MINIMIZE):
		/* Handle a minimize call by supressing rendering. */
		shouldSuppressRender = true;
		break;
	case (SC_RESTORE):
		shouldSuppressRender = false;
		break;
	}

	return DefWindowProc(hndl, WM_SYSCOMMAND, wParam, lParam);
}

LRESULT Pu::Win32Window::HandleInput(WPARAM wParam, LPARAM lParam)
{
	constexpr UINT ERROR_CODE = static_cast<UINT>(-1);
	const HRAWINPUT handle = reinterpret_cast<HRAWINPUT>(lParam);

	/* Only handle foreground events. */
	if (!GET_RAWINPUT_CODE_WPARAM(wParam))
	{
		/* Get the size of this data package (should never fail). */
		uint32 size = 0;
		GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

		/* Early out for empty messages. */
		if (size)
		{
			if (size > rawInputSize)
			{
				/* Increase the buffer size if needed, to avoid allocating on every WM_INPUT. */
				rawInputSize = size;
				input = reinterpret_cast<PRAWINPUT>(realloc(input, size));
			}

			/* Get the header information, size should be passed but never changes (should never fail). */
			GetRawInputData(handle, RID_HEADER, input, &size, sizeof(RAWINPUTHEADER));

			/* Ignore the input is the device is not a valid HID. */
			if (input->header.hDevice)
			{
				/* Get the actual data from the event. */
				GetRawInputData(handle, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));

				/* Post the event to any listeners. */
				OnInputEvent.Post(*this, *input);
			}
		}
	}

	return DefWindowProc(hndl, WM_INPUT, wParam, lParam);
}
#endif