#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines a vertex layout that stores a positionand normal. */
	struct Terrain
	{
	public:
		/* The location of the vertex. */
		Vector3 Position;
		/* The surface normal of the vertex. */
		Vector3 Normal;
		uint32 PlateId;
	};
}