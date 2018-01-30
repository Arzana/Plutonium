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
		int Width;
		/* The physical height of the monitor. */
		int Height;

		/* Gets the relative position of the monitor. */
		_Check_return_ Vector2 GetPosition(void) const;
		/* Gets the size of the monitor. */
		_Check_return_ Vector2 GetSize(void) const;
		/* Gets a rectangle that defines the physical bounds of the monitor. */
		_Check_return_ Rectangle GetBounds(void) const;

		/* Gets the monitor specified as the primary monitor by the operating system. */
		_Check_return_ static MonitorInfo GetPrimary(void);
		/* Gets all the monitors that are defined by the operating system. */
		_Check_return_ static std::vector<MonitorInfo> GetAll(void);
		/* Gets the monitor on which the specified window is displayed. */
		_Check_return_ static MonitorInfo FromWindow(_In_ GLFWwindow *hndlr);

	private:
		MonitorInfo(GLFWmonitor *info);
	};
}