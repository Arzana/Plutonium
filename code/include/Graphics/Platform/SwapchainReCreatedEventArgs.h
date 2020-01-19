#pragma once
#include <sal.h>
#include "Core/Events/EventArgs.h"

namespace Pu
{
	/* Defines the event arguments for when a swapchain is re-created by a GameWindow. */
	struct SwapchainReCreatedEventArgs
		: public EventArgs
	{
	public:
		/* Specifies whether the size of the game window was changed. */
		bool AreaChanged;
		/* Specifies whether the image format of the swapchain was changed. */
		bool FormatChanged;
		/* Specifies whether the swapchain no longer has access to the surface. */
		bool SurfaceLost;

		/* Initializes a new instance of a swapchain re-create event args object. */
		SwapchainReCreatedEventArgs(_In_ bool area, _In_ bool format, _In_ bool surface)
			: AreaChanged(area), FormatChanged(format), SurfaceLost(surface)
		{}
	};
}