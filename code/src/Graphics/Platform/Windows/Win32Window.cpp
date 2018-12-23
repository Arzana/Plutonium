#ifdef _WIN32
#include "Graphics/Platform/Windows/Win32Window.h"
#include "Graphics/Vulkan/Surface.h"
#include "Core/Diagnostics/DbgUtils.h"

static Pu::vector<Pu::Win32Window*> activeWindows;

static constexpr int GetLowWord(LPARAM lParam)
{
	return static_cast<int>(static_cast<short>(LOWORD(lParam)));
}

static constexpr int GetHighWord(LPARAM lParam)
{
	return static_cast<int>(static_cast<short>(HIWORD(lParam)));
}

Pu::Win32Window::Win32Window(VulkanInstance & vulkan, const char * title, Vector2 size)
	: NativeWindow(), title(title), vp(size.X, size.Y), mode(WindowMode::Windowed), shouldClose(false), AllowAltF4(true), focused(false)
{
	/* Push this window as an active window. */
	activeWindows.push_back(this);

	/* Create new module handle for this module. */
	instance = GetModuleHandle(nullptr);

	/* Create window creation information. */
	WNDCLASSEX wndEx =
	{
		sizeof(WNDCLASSEX),					// size
		CS_HREDRAW | CS_VREDRAW,			// style
		Win32Window::WndProc,				// proc
		0,									// cls extra
		0,									// wnd extra
		instance,							// instance
		nullptr,							// icon
		LoadCursor(nullptr, IDC_ARROW),		// cursor
		(HBRUSH)(COLOR_WINDOW + 1),			// background 
		nullptr,							// menu name
		title,								// class name
		nullptr								// icon sm
	};

	/* Register new window class. */
	if (!RegisterClassEx(&wndEx))
	{
		const string error = _CrtGetErrorString();
		Log::Fatal("Unable to register Win32 window (%s)!", error.c_str());
	}

	/* Create new window. */
	hndl = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, 0, 0, ipart(vp.Width), ipart(vp.Height), nullptr, nullptr, instance, nullptr);
	if (!hndl)
	{
		const string error = _CrtGetErrorString();
		Log::Fatal("Unable to create Win32 window (%s)!", error.c_str());
	}

	/* Create new surface. */
	surface = new Surface(vulkan, instance, hndl);
}

Pu::Win32Window::~Win32Window(void)
{
	/* Destroy handles. */
	delete surface;
	if (hndl) DestroyWindow(hndl);
	if (instance) UnregisterClass(title, instance);

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

void Pu::Win32Window::Show(void)
{
	ShowWindow(hndl, SW_SHOWNORMAL);
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
}

bool Pu::Win32Window::Update(void)
{
	/* Call WM_PAINT for window. */
	if (!UpdateWindow(hndl)) Log::Warning("Win32 window update has failed!");

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
		if (window->hndl == hWnd) return window->HandleProc(message, wParam, lParam);
	}

	/* 
	Only log an unknow warning if it's not one of the specified messages.
	These messages occur before the window handle is returned and thusly we cannot match it to a window yet.
	- WM_CREATE:		Sent to incidate a new window to be created by CreateWindowEx or CreateWindow.
	- WM_GETMINMAXINFO:	Sent when the size or positio of a window is about to change.
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
	pos = args.NewValue;
	OnLocationChanged.Post(*this, args);
}

void Pu::Win32Window::Resize(int w, int h)
{
	ValueChangedEventArgs args(Vector2(vp.Width, vp.Height), Vector2(static_cast<float>(w), static_cast<float>(h)));
	vp.Width = args.NewValue.X;
	vp.Height = args.NewValue.Y;
	OnSizeChanged.Post(*this, args);
}

void Pu::Win32Window::UpdateClientArea(void)
{
	if (SetWindowPos(hndl, HWND_TOP, ipart(pos.X), ipart(pos.Y), ipart(vp.Width), ipart(vp.Height), 0))
	{
		Log::Warning("Win32 window client area update failed!");
	}
}

LRESULT Pu::Win32Window::HandleProc(UINT message, WPARAM wParam, LPARAM lParam)
{
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
		focused = true;
		return 0;
	case (WM_KILLFOCUS):
		focused = false;
		return 0;
	case (WM_ACTIVATE):
		focused = GetLowWord(wParam) != WA_INACTIVE;
		return 0;
	case (WM_SYSCOMMAND):
		return HandleSysCmd(wParam, lParam);
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
	}

	return DefWindowProc(hndl, WM_SYSCOMMAND, wParam, lParam);
}
#endif