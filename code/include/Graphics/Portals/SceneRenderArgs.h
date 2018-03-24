#pragma once
#include "Graphics\Portals\Portal.h"

namespace Plutonium
{
	/* Defines the arguments for a scene render call by a portal renderer. */
	struct SceneRenderArgs
	{
		/* The identifier of the scene to be rendered. */
		int32 SceneID;
		/* The view matrix that should be used to render the scene. */
		Matrix View;
		/* The projection matrix that should be used to render the scene. */
		Matrix Projection;
	};

	/* Defines the argument for a portal addition to a portal renderer. */
	struct PortalRenderArgs
	{
		/* The identifier of the destination scene. */
		int32 SceneID;
		/* The portal to the destination scene. */
		Portal *Portal;
	};
}