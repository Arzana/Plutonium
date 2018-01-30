#include "Graphics\Native\Monitor.h"
#include "Core\Logging.h"
#include <glfw3.h>

using namespace Plutonium;

Vector2 Plutonium::MonitorInfo::GetPosition(void) const
{
	return Vector2(static_cast<float>(X), static_cast<float>(Y));
}

Vector2 Plutonium::MonitorInfo::GetSize(void) const
{
	return Vector2(static_cast<float>(Width), static_cast<float>(Height));
}

Rectangle Plutonium::MonitorInfo::GetBounds(void) const
{
	return Rectangle(GetPosition(), GetSize());
}

MonitorInfo Plutonium::MonitorInfo::GetPrimary(void)
{
	return MonitorInfo(glfwGetPrimaryMonitor());
}

std::vector<MonitorInfo> Plutonium::MonitorInfo::GetAll(void)
{
	/* Get underlying monitor pointers. */
	int displayCnt = 0;
	GLFWmonitor **displays = glfwGetMonitors(&displayCnt);

	/* Populate result. */
	std::vector<MonitorInfo> result;
	for (size_t i = 0; i < displayCnt; i++)
	{
		result.push_back(MonitorInfo(displays[i]));
	}

	return result;
}

MonitorInfo Plutonium::MonitorInfo::FromWindow(GLFWwindow * hndlr)
{
	/* Get window position vector. */
	int x = 0, y = 0;
	glfwGetWindowPos(hndlr, &x, &y);
	Vector2 pos = Vector2(static_cast<float>(x), static_cast<float>(y));

	/* Loop through available monitors to find result. */
	std::vector<MonitorInfo> monitors = GetAll();
	for (size_t i = 0; i < monitors.size(); i++)
	{
		const MonitorInfo cur = monitors.at(i);
		if (cur.GetBounds().Contains(pos)) return cur;
	}

	/* If no monitor was found or window is outside of the monitor bounds return the primary instead. */
	LOG_WAR("Cannot get monitor for window at position (%d, %d), returning primary monitor!", x, y);
	return GetPrimary();
}

Plutonium::MonitorInfo::MonitorInfo(GLFWmonitor * info)
	: Name(glfwGetMonitorName(info))
{
	glfwGetMonitorPos(info, &X, &Y);
	glfwGetMonitorPhysicalSize(info, &Width, &Height);
}