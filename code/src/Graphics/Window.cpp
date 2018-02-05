#include "Graphics\Window.h"
#include "Graphics\Native\OpenGL.h"
#include "Graphics\Native\Monitor.h"
#include <glfw3.h>

using namespace Plutonium;

std::vector<Window*> activeWnds;
Window * Plutonium::GetWndFromHndlr(GLFWwindow *hndlr)
{
	/* Attempt to find the propper window. */
	for (size_t i = 0; i < activeWnds.size(); i++)
	{
		Window *cur = activeWnds.at(i);
		if (cur->hndlr == hndlr) return cur;
	}

	/* Should never occur. */
	LOG_WAR("Unknown window requested!");
	return nullptr;
}

void GlfwSizeChangedEventHandler(GLFWwindow *hndlr, int w, int h)
{
	/* Get window associated with handler and call resize. */
	Window *wnd = GetWndFromHndlr(hndlr);
	if (wnd) wnd->Resize(Vector2(static_cast<float>(w), static_cast<float>(h)));
}

void GlfwPositionChangedEventHandler(GLFWwindow *hndlr, int x, int y)
{
	/* Get window associated and call move. */
	Window *wnd = GetWndFromHndlr(hndlr);
	if (wnd) wnd->Move(Vector2(static_cast<float>(x), static_cast<float>(y)));
}

void Plutonium::GlfwFocusChangedEventHandler(GLFWwindow *hndlr, int gainedFocus)
{
	/* Get window associated. */
	Window *wnd = GetWndFromHndlr(hndlr);
	if (wnd)
	{
		if (gainedFocus == GLFW_TRUE)
		{
			/* Apply focus gain. */
			wnd->focused = true;
			wnd->GainedFocus.Post(wnd, EventArgs());
		}
		else
		{
			/* Apply focus loss. */
			wnd->focused = false;
			wnd->LostFocus.Post(wnd, EventArgs());
		}
	}
}

int CreateNewWindow(GLFWwindow **hndlr, int w, int h, const char *title)
{
	/* Set OpenGL requirements, 4.3 is needed for logging. */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Make sure GLFW is initialized. */
	if (_CrtInitGLFW() != GLFW_TRUE) return GLFW_FALSE;

	/* Create new window handler. */
	*hndlr = glfwCreateWindow(w, h, title, nullptr, nullptr);
	if (!*hndlr)
	{
		LOG_THROW("Failed to create new window '%s'!", title);
		return GLFW_FALSE;
	}

	glfwMakeContextCurrent(*hndlr);

	/* Set GLFW specific event handlers. */
	glfwSetWindowSizeCallback(*hndlr, GlfwSizeChangedEventHandler);
	glfwSetWindowPosCallback(*hndlr, GlfwPositionChangedEventHandler);
	glfwSetWindowFocusCallback(*hndlr, Plutonium::GlfwFocusChangedEventHandler);

	/* Log results. */
	LOG("Created new window '%s'(%dx%d).", title, w, h);
	return GLFW_TRUE;
}

Plutonium::Window::Window(const char * title, Vector2 size)
	: title(title), wndBounds(size), vpBounds(size), focused(false), wndMode(WindowMode::Windowed), swapMode(VSyncMode::Enabled),
	SizeChanged("WindowSizeChanged"), PositionChanged("WindowPositionChanged"), GainedFocus("WindowGainedFocus"), LostFocus("WindowLostFocus"),
	operational(false)
{
	/* Create underlying window. */
	if (CreateNewWindow(&hndlr, static_cast<int>(size.X), static_cast<int>(size.Y), title) != GLFW_TRUE) return;
	activeWnds.push_back(this);

	/* Make sure OpenGL is initialized. */
	if (_CrtInitGlad() != GLFW_TRUE) return;

	/* Initialize properties and show window. */
	SetBounds(Vector2::Zero, size);
	Show();
	operational = true;
}

Plutonium::Window::~Window(void)
{
	/* Remove window from active list. */
	for (size_t i = 0; i < activeWnds.size(); i++)
	{
		WindowHandler cur = activeWnds.at(i);
		if (cur == this)
		{
			activeWnds.erase(activeWnds.begin() + i);
			break;
		}
	}

	/* Release underlying window handler. */
	if (hndlr) glfwDestroyWindow(hndlr);
	if (activeWnds.size() < 1) _CrtFinalizeGLFW();
}

void Plutonium::Window::Show(void) const
{
	/* Make sure we don't show an already shown window. */
	if (focused) return;
	focused = true;

#if defined(DEBUG)
	_CrtDbgMoveTerminal(hndlr);
#endif

	glfwShowWindow(hndlr);
}

void Plutonium::Window::Hide(void) const
{
	/* Make sure we don't hide an already hidden window. */
	if (!focused) return;
	focused = false;

	glfwHideWindow(hndlr);
}

void Plutonium::Window::Close(void) const
{
	glfwSetWindowShouldClose(hndlr, GLFW_TRUE);
}

void Plutonium::Window::Resize(Vector2 size)
{
	/* Check if input is valid. */
	if (size.X <= 0.0f || size.Y <= 0.0f)
	{
		LOG_WAR("Cannot set window size to non positive value (%f, %f)!", size.X, size.Y);
		return;
	}

	SetBounds(wndBounds.Position, size);
}

void Plutonium::Window::Move(Vector2 position)
{
	/* Calculate full display bounds. */
	Rectangle bounds;
	std::vector<MonitorInfo> displays = MonitorInfo::GetAll();
	for (size_t i = 0; i < displays.size(); i++) bounds = Rectangle::Merge(bounds, displays.at(i).GetWindowBounds());

	/* Check if position is within the display bounds. */
	if (!bounds.Contains(position))
	{
		LOG_WAR("Cannot place window outside of display bounds!");
		return;
	}

	/* Set new position. */
	SetBounds(position, vpBounds.Size);
}

void Plutonium::Window::SetMode(WindowMode mode)
{
	/* Check if input isn't equal to the current mode. */
	if (wndMode == mode) return;

	/* Get current display. */
	MonitorInfo display = MonitorInfo::FromWindow(hndlr);
	if (!display.IsValid)
	{
		LOG_WAR("Could not get display associated with window '%s'!", title);
		return;
	}

	/* Get current window attributes. */
	int x = static_cast<int>(vpBounds.Size.X);
	int y = static_cast<int>(vpBounds.Size.Y);
	int w = static_cast<int>(vpBounds.GetWidth());
	int h = static_cast<int>(vpBounds.GetHeight());

	switch (mode)
	{
		case Plutonium::WindowMode::Windowed:
			/* Just set the monitor to NULL and keep everything else the same. */
			glfwSetWindowMonitor(hndlr, nullptr, x, y, w, h, display.RefreshRate);
			break;
		case Plutonium::WindowMode::BorderlessFullscreen:
			/* Set the monitor to the specified current monitor and resize the window. */
			glfwSetWindowMonitor(hndlr, display.Handle, 0, 0, display.ClientWidth, display.ClientHeight, display.RefreshRate);
			SetBounds(Vector2::Zero, display.GetClientSize());
			break;
		case Plutonium::WindowMode::Fullscreen:
			/* Set the monitor to the specified current monitor. */
			glfwSetWindowMonitor(hndlr, display.Handle, 0, 0, w, h, display.RefreshRate);
			SetBounds(Vector2::Zero, vpBounds.Size);
			break;
		default:
			LOG_WAR("Attempted to set unsupported window mode!");
			return;
	}

	/* Reset swap interval is needed for fullscreen window modes. */
	SetVerticalRetrace(swapMode);
	wndMode = mode;
}

void Plutonium::Window::SetMode(VSyncMode mode)
{
	/* Check if input isn't equal to the current mode. */
	if (swapMode == mode) return;

	/* Update window setting. */
	SetVerticalRetrace(mode);
	swapMode = mode;
}

void Plutonium::Window::SetVerticalRetrace(VSyncMode mode)
{
	switch (mode)
	{
		case Plutonium::VSyncMode::Enabled:
			glfwSwapInterval(1);
			break;
		case Plutonium::VSyncMode::Disable:
			glfwSwapInterval(0);
			break;
		case Plutonium::VSyncMode::DisableForce:
			_CrtSetSwapIntervalExt(0);
			break;
		default:
			LOG_WAR("Attempted to set unsupported vertical retrace mode!");
			return;
	}
}

bool Plutonium::Window::Update(void)
{
	/* Make sure events are updated (mouse, keyboard, etc.) and return whether the window should be finalized. */
	glfwPollEvents();
	return glfwWindowShouldClose(hndlr);
}

void Plutonium::Window::SetBounds(Vector2 pos, Vector2 size)
{
	/* Check if window was moved. */
	if (pos != wndBounds.Position)
	{
		wndBounds.Position = pos;
		PositionChanged.Post(this, EventArgs());
		LOG("Window '%s' moved to (%dx%d).", title, static_cast<int>(pos.X), static_cast<int>(pos.Y));
	}

	/* Check if window was resized. */
	if (size != vpBounds.Size)
	{
		vpBounds.Size = size;

		/* Check if viewport needs to be updated. */
		int l = 0, r = 0, t = 0, b = 0;
		glfwGetWindowFrameSize(hndlr, &l, &t, &r, &b);
		wndBounds.Size = vpBounds.Size + Vector2(static_cast<float>(l + r), static_cast<float>(t + b));

		SizeChanged.Post(this, EventArgs());
		LOG("Window '%s' resized to %dx%d.", title, static_cast<int>(size.X), static_cast<int>(size.Y));
	}
}