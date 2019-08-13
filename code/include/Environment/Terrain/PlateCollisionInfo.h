#pragma once
#include "Core/Math/Point.h"

namespace Pu
{
	class Plate;

	/* Defines the information required to handle a collision between two plates. */
	struct PlateCollisionInfo
	{
	public:
		/* Initializes a new instance of a plate collision info object. */
		PlateCollisionInfo(_In_ Plate &first, _In_ Plate &second, _In_ LSize pos, _In_ float crust)
			: first(&first), second(&second), pos(pos), crust(crust)
		{}

		/* Gets the first plate in this collision. */
		_Check_return_ Plate& GetFirst(void) const
		{
			return *first;
		}

		/* Gets the second plate in this collision. */
		_Check_return_ Plate& GetSecond(void) const
		{
			return *second;
		}

		/* Gets the world position of the collision. */
		_Check_return_ LSize GetPosition(void) const
		{
			return pos;
		}

		/* Gets the amount of crust that will be deformed or subducted. */
		_Check_return_ float GetCrush(void) const
		{
			return crust;
		}

	private:
		Plate *first, *second;
		LSize pos;
		float crust;
	};
}