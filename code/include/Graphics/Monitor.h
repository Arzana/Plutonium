#pragma once
#include "Core\Math\Rectangle.h"

struct GLFWmonitor;
struct GLFWwindow;

namespace Plutonium
{
	/* Contains all visible information about a specific physical monitor. */
	struct MonitorInfo
	{
	public:
		const char *Name;
		int X;
		int Y;
		int Width;
		int Height;

		_Check_return_ Vector2 GetPosition(void) const;
		_Check_return_ Vector2 GetSize(void) const;
		_Check_return_ Rectangle GetBounds(void) const;

		_Check_return_ static const MonitorInfo GetPrimary(void) const;
		_Check_return_ static const MonitorInfo* GetAll(size_t *count);
		_Check_return_ static const MonitorInfo FromWindow(_In_ GLFWwindow *hndlr);

	private:
		MonitorInfo(GLFWmonitor *info);
	};
}