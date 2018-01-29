#include "Graphics\Window.h"
#include "Graphics\Native\OpenGL.h"
#include "glfw3.h"
#include <vector>

using namespace Plutonium;

std::vector<Window*> activeWnds;
Window * Plutonium::GetWndFromHndlr(GLFWwindow *hndlr)
{
	for (size_t i = 0; i < activeWnds.size(); i++)
	{
		Window *cur = activeWnds.at(i);
		if (cur->hndlr == hndlr) return cur;
	}

	LOG_WAR("Unknown window requested!");
	return nullptr;
}

void GlfwSizeChangedEventHandler(GLFWwindow *hndlr, int w, int h)
{
	Window *wnd = GetWndFromHndlr(hndlr);
	if (wnd) wnd->Resize(Vector2(static_cast<float>(w), static_cast<float>(h)));
}

void GlfwPositionChangedEventHandler(GLFWwindow *hndlr, int x, int y)
{
	Window *wnd = GetWndFromHndlr(hndlr);
	if (wnd) wnd->Move(Vector2(static_cast<float>(x), static_cast<float>(y)));
}

void Plutonium::GlfwFocusChangedEventHandler(GLFWwindow *hndlr, int gainedFocus)
{
	Window *wnd = GetWndFromHndlr(hndlr);
	if (wnd)
	{
		if (gainedFocus == GLFW_TRUE)
		{
			wnd->focused = true;
			wnd->GainedFocus.Post(wnd, EventArgs());
		}
		else
		{
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
		LOG_ERR("Failed to create new window '%s'!", title);
		return GLFW_FALSE;
	}

	/* Set GLFW specific event handlers. */
	glfwMakeContextCurrent(*hndlr);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(*hndlr, GlfwSizeChangedEventHandler);
	glfwSetWindowPosCallback(*hndlr, GlfwPositionChangedEventHandler);
	glfwSetWindowFocusCallback(*hndlr, Plutonium::GlfwFocusChangedEventHandler);

	/* Log results. */
	LOG("Created new window '%s'(%dx%d).", title, w, h);
	return GLFW_TRUE;
}

Plutonium::Window::Window(const char * title, Vector2 size)
	: title(title), wndBounds(size), vpBounds(size), focused(false), mode(WindowMode::Windowed),
	SizeChanged("WindowSizeChanged"), GainedFocus("WindowGainedFocus"), LostFocus("WindowLostFocus"),
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

	/* Check if input isn't equal to the current size. */
	if (size.X != vpBounds.GetWidth() && size.Y != vpBounds.GetHeight())
	{
		SetBounds(vpBounds.Position, size);
		LOG("Window '%s' resized to %dx%d.", title, static_cast<int>(size.X), static_cast<int>(size.Y));
		SizeChanged.Post(this, EventArgs());
	}
}

void Plutonium::Window::Move(Vector2 position)
{
	/* Check if input isn't equal to the current position. */
	if (position == wndBounds.Position) return;

	/* Get all displays. */
	int displayCnt = 0;
	GLFWmonitor **displays = glfwGetMonitors(&displayCnt);

	/* Create valid area bounds. */
	Rectangle bounds = Rectangle();
	for (size_t i = 0; i < displayCnt; i++)
	{
		int x = 0, y = 0, w = 0, h = 0;
		glfwGetMonitorPos(displays[i], &x, &y);
		glfwGetMonitorPhysicalSize(displays[i], &w, &h);
		Rectangle cur = Rectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
		bounds = Rectangle::Merge(bounds, cur);
	}

	/* Check if position is within the display bounds. */
	if (!bounds.Contains(position))
	{
		LOG_WAR("Cannot place window outside of display bounds!");
		return;
	}

	/* Set new position. */
	SetBounds(position, vpBounds.Size);
	LOG("Window '%s' moved to %dx%d.", title, static_cast<int>(position.X), static_cast<int>(position.Y));
}

void Plutonium::Window::SetMode(WindowMode mode)
{
	/* Check if input isn't equal to the current mode. */
	if (this->mode == mode) return;


}