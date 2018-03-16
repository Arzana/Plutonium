#include "Input\Keyboard.h"
#include "Graphics\Native\Window.h"
#include "glfw3.h"
#include <vector>

using namespace Plutonium;

std::vector<Keyboard*> activeKeyboards;
Keyboard * Plutonium::GetKeyboardFromHndlr(GLFWwindow *hndlr)
{
	/* Get keyboard associated with handle. */
	for (size_t i = 0; i < activeKeyboards.size(); i++)
	{
		Keyboard *cur = activeKeyboards.at(i);
		if (cur->wnd->hndlr == hndlr) return cur;
	}

	/* Should never occur. */
	LOG_WAR("Unknown keyboard requested!");
	return nullptr;
}

void Plutonium::GlfwKeyInputEventHandler(GLFWwindow *hndlr, int key, int scanCode, int action, int mods)
{
	/* Get keyboard associated with handle and post event. */
	Keyboard *keyboard = GetKeyboardFromHndlr(hndlr);
	if (keyboard) keyboard->KeyPress.Post(keyboard->wnd, KeyEventArgs(key, scanCode, action, mods));
}

void Plutonium::GlfwCharInputEventHandler(GLFWwindow *hndlr, uint32 codepoint)
{
	/* Get keyboard associated with handle and post event. */
	Keyboard *keyboard = GetKeyboardFromHndlr(hndlr);
	if (keyboard) keyboard->CharInput.Post(keyboard->wnd, codepoint);
}

Plutonium::Keyboard::~Keyboard(void)
{
	for (size_t i = 0; i < activeKeyboards.size(); i++)
	{
		if (activeKeyboards.at(i) == this)
		{
			activeKeyboards.erase(activeKeyboards.begin() + i);
			return;
		}
	}
}

bool Plutonium::Keyboard::IsKeyDown(Keys key) const
{
	return GetKey(key) != KeyState::Up;
}

bool Plutonium::Keyboard::IsKeyUp(Keys key) const
{
	return GetKey(key) == KeyState::Up;
}

KeyState Plutonium::Keyboard::GetKey(Keys key) const
{
	return static_cast<KeyState>(glfwGetKey(wnd->hndlr, static_cast<int>(key)));
}

Plutonium::Keyboard::Keyboard(const Window * wnd)
	: wnd(wnd), KeyPress("KeyboardKeyPress"), CharInput("KeyboardCharInput")
{
	activeKeyboards.push_back(this);

	/* Set GLFW specific event handlers. */
	glfwSetKeyCallback(wnd->hndlr, GlfwKeyInputEventHandler);
	glfwSetCharCallback(wnd->hndlr, GlfwCharInputEventHandler);
}
