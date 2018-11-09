#ifdef _WIN32
#include "Graphics/Platform/Windows/Win32Window.h"
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
	: NativeWindow(), title(title), vp(size), mode(WindowMode::Windowed), shouldClose(false), AllowAltF4(true), focused(false)
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
	hndl = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, 0, 0, ipart(vp.GetWidth()), ipart(vp.GetHeight()), nullptr, nullptr, instance, nullptr);
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
	delete_s(surface);
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
	vp.Size = newSize;
	UpdateClientArea();
}

void Pu::Win32Window::Move(Vector2 newLocation)
{
	vp.Position = newLocation;
	UpdateClientArea();
}

void Pu::Win32Window::SetMode(WindowMode mode)
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

	Log::Warning("Unknown window received at WndProc!");
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Pu::Win32Window::Move(int x, int y)
{
	ValueChangedEventArgs args(vp.Position, Vector2(static_cast<float>(x), static_cast<float>(y)));
	vp.Position = args.NewValue;
	OnLocationChanged.Post(*this, args);
}

void Pu::Win32Window::Resize(int w, int h)
{
	ValueChangedEventArgs args(vp.Size, Vector2(static_cast<float>(w), static_cast<float>(h)));
	vp.Size = args.NewValue;
	OnSizeChanged.Post(*this, args);
}

void Pu::Win32Window::UpdateClientArea(void)
{
	if (SetWindowPos(hndl, HWND_TOP, ipart(vp.Position.X), ipart(vp.Position.Y), ipart(vp.GetWidth()), ipart(vp.GetHeight()), 0))
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