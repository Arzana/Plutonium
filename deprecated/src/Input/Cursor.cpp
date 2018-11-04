#include "Input\Cursor.h"
#include "Graphics\Native\Window.h"
#include <glfw3.h>
#include <vector>

using namespace Plutonium;

std::vector<Cursor*> activeCursors;
Cursor * Plutonium::GetCursorFromHndlr(GLFWwindow *hndlr)
{
	/* Get cursor associated with handle. */
	for (size_t i = 0; i < activeCursors.size(); i++)
	{
		Cursor *cur = activeCursors.at(i);
		if (cur->wnd->hndlr == hndlr) return cur;
	}

	/* Should never occur. */
	LOG_WAR("Unknown cursor requested!");
	return nullptr;
}

void Plutonium::GlfwCursorFocusEventHandler(GLFWwindow *hndlr, int entered)
{
	/* Get cursor associated with handle. */
	Cursor *cursor = GetCursorFromHndlr(hndlr);
	if (cursor)
	{
		/* Call correct enter/leave event. */
		if (entered) cursor->EnterWindow.Post(cursor->wnd, cursor);
		else cursor->LeaveWindow.Post(cursor->wnd, cursor);
	}
}

void Plutonium::GlfwCursorButtonEventHandler(GLFWwindow *hndlr, int btn, int action, int mods)
{
	/* Get cursor associated with handle. */
	Cursor *cursor = GetCursorFromHndlr(hndlr);
	if (cursor)
	{
		/* Set correct button to the correct state. */
		switch (btn)
		{
			case (GLFW_MOUSE_BUTTON_LEFT):
				cursor->LeftButton = action == GLFW_PRESS;
				break;
			case (GLFW_MOUSE_BUTTON_RIGHT):
				cursor->RightButton = action == GLFW_PRESS;
				break;
			case (GLFW_MOUSE_BUTTON_MIDDLE):
				cursor->MiddleButton = action == GLFW_PRESS;
				break;
			default:
				/* On unhandled key we call the event and let the user do the work. */
				cursor->SpecialButtonPress.Post(cursor->wnd, cursor, ButtonEventArgs(btn, action == GLFW_PRESS, mods));
				break;
		}
	}
}

void Plutonium::GlfwCursorMoveEventHandler(GLFWwindow *hndlr, double x, double y)
{
	/* Get cursor associated with handle and change cursor position. */
	Cursor *cursor = GetCursorFromHndlr(hndlr);
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);

	if (cursor)
	{
		/* Make sure we don't update the delta position on the first move. */
		if (cursor->firstMovement) cursor->firstMovement = false;
		else
		{
			/* Update delta movement. */
			cursor->DeltaX += ix - cursor->X;
			cursor->DeltaY += iy - cursor->Y;
		}

		/* Update to new position. */
		cursor->X = ix;
		cursor->Y = iy;
	}
}

void GlfwCursorScrollEventHandler(GLFWwindow *hndlr, double x, double y)
{
	/* Get cursor associated with handle. */
	Cursor *cursor = GetCursorFromHndlr(hndlr);
	if (cursor)
	{
		/* Apply offset to the cursor. */
		cursor->ScrollWheel.X += static_cast<float>(x);
		cursor->ScrollWheel.Y += static_cast<float>(y);
	}
}

Plutonium::Cursor::~Cursor(void)
{
	for (size_t i = 0; i < activeCursors.size(); i++)
	{
		if (activeCursors.at(i) == this)
		{
			activeCursors.erase(activeCursors.begin() + i);
			return;
		}
	}
}

Vector2 Plutonium::Cursor::GetPosition(void) const
{
	return Vector2(static_cast<float>(X), static_cast<float>(Y));
}

Vector2 Plutonium::Cursor::GetDelta(void) const
{
	return Vector2(static_cast<float>(DeltaX), static_cast<float>(DeltaY));
}

bool Plutonium::Cursor::IsVisible(void) const
{
	return glfwGetInputMode(wnd->hndlr, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void Plutonium::Cursor::SetPosition(Vector2 position)
{
	glfwSetCursorPos(wnd->hndlr, static_cast<double>(position.X), static_cast<double>(position.Y));
}

void Plutonium::Cursor::Disable(void)
{
	glfwSetInputMode(wnd->hndlr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Plutonium::Cursor::Hide(void)
{
	glfwSetInputMode(wnd->hndlr, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void Plutonium::Cursor::Show(void)
{
	glfwSetInputMode(wnd->hndlr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	UpdatePostion();
}

Plutonium::Cursor::Cursor(const Window * wnd)
	: wnd(wnd), EnterWindow("CursorEnterWindow"), LeaveWindow("CursorLeaveWindow"), SpecialButtonPress("CursorSpecialButtonPress"),
	X(0), Y(0), LeftButton(false), RightButton(false), MiddleButton(false), firstMovement(true)
{
	activeCursors.push_back(this);
	Reset();

	/* Set GLFW specific event handlers. */
	glfwSetCursorPosCallback(wnd->hndlr, GlfwCursorMoveEventHandler);
	glfwSetCursorEnterCallback(wnd->hndlr, GlfwCursorFocusEventHandler);
	glfwSetMouseButtonCallback(wnd->hndlr, GlfwCursorButtonEventHandler);
	glfwSetScrollCallback(wnd->hndlr, GlfwCursorScrollEventHandler);

	/* Set the position for the first tick. */
	UpdatePostion();
}

void Plutonium::Cursor::Reset(void)
{
	/* Reset scrollwheel delta and movement. */
	ScrollWheel = Vector2::Zero();
	DeltaX = 0;
	DeltaY = 0;
}

void Plutonium::Cursor::UpdatePostion(void)
{
	double x, y;
	glfwGetCursorPos(wnd->hndlr, &x, &y);
	X = static_cast<int>(x);
	Y = static_cast<int>(y);
}