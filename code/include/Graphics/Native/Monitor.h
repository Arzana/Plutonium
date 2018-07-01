#pragma once
#include <vector>
#include "Core\Math\Rectangle.h"

struct GLFWmonitor;
struct GLFWwindow;

namespace Plutonium
{
	/* Contains all visible information about a specific physical monitor. */
	struct MonitorInfo
	{
	public:
		/* Specifies the name given to the monitor. */
		const char *Name;
		/* The relative horizontal position of the monitor. */
		int X;
		/* The relative vertical position of the monitor. */
		int Y;
		/* The physical width of the monitor. */
		int WindowWidth;
		/* The physical height of the monitor. */
		int WindowHeight;
		/* The viewport width of the monitor. */
		int ClientWidth;
		/* The viewport height of the monitor. */
		int ClientHeight;
		/* The bit depth of the red channel. */
		int Red;
		/* The bit depth of the green channel. */
		int Green;
		/* The bit depth of the blue channel. */
		int Blue;
		/* The refresh rate int Hz. */
		int RefreshRate;
		/* The average gamma correction that should be applied to the final fragment color. */
		float GammaCorrection;
		/* The handler for the monitor. */
		GLFWmonitor *Handle;
		/* Whether this structure is valid. */
		bool IsValid;

		/* Gets the relative position of the monitor. */
		_Check_return_ Vector2 GetPosition(void) const;
		/* Gets the phycical size of the monitor. */
		_Check_return_ Vector2 GetWindowSize(void) const;
		/* Gets the viewport size of the monitor. */
		_Check_return_ Vector2 GetClientSize(void) const;
		/* Gets a rectangle that defines the physical bounds of the monitor. */
		_Check_return_ Rectangle GetWindowBounds(void) const;
		/* Gets a rectangle that defines the viewport bounds of the monitor. */
		_Check_return_ Rectangle GetClientBounds(void) const;

		/* Gets the monitor specified as the primary monitor by the operating system. */
		_Check_return_ static MonitorInfo GetPrimary(void);
		/* Gets all the monitors that are defined by the operating system. */
		_Check_return_ static std::vector<MonitorInfo>& GetAll(void);
		/* Gets the monitor on which the specified window is displayed. */
		_Check_return_ static MonitorInfo FromWindow(_In_ GLFWwindow *hndlr);

	private:
		MonitorInfo(GLFWmonitor *info);
	};
}