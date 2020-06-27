#pragma once
#include "Core/Math/Matrix.h"
#include "Core/Collections/fvector.h"

namespace Pu
{
	/* Defines a cache store used by the physics engine. */
	class PhysicsCache
	{
	public:

	private:
		vector<Matrix> staticTransforms;
		fvector qx;
		fvector qy;
		fvector qz;
	};
}