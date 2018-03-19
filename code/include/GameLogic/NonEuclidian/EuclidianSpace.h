#pragma once
#include "Graphics\Models\StaticModel.h"

namespace Plutonium
{
	/* Defines a convex Euclidian space. */
	struct EuclidianSpace
	{
	public:
		/* Gets the position of the space to be used in a physical world. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return pos;
		}

		/* Gets the gravitational force active in this space. */
		_Check_return_ inline Vector3 GetGravity(void) const
		{
			return gravity;
		}

	private:
		/* Defines the basic information of a portal to another room. */
		struct Portal
		{
			/* The desination room. */
			EuclidianSpace *dest;
			/* The amount of vertices defined by the portal bounds mesh. */
			size_t bndsVrtxCnt;
			/* The bounds mesh of the portal. */
			Vector3 *bounds;
		};

		Vector3 pos;
		Vector3 gravity;
		StaticModel *model;
		std::vector<Portal*> portals;
	};
}