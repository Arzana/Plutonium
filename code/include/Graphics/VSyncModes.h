#pragma once

namespace Plutonium
{
	/* Defines the options for how a window whould handle synchronization with vertical retrace. */
	enum class VSyncMode
	{
		/* Enable synchonization with vertical retrace. */
		Enabled,
		/* Disable GLFW's back buffer swap interval. */
		Disable,
		/* Disable GLFW's back buffer swap interval and force the OS to disable synchonization with vertical retrace. */
		DisableForce
	};
}