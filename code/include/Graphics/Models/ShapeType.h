#pragma once
#include <sal.h>

namespace Pu
{
	/* Defines the basic mesh types supported for easy creation. */
	enum class ShapeType
	{
		/* Defines a 3D plane. */
		Plane,
		/* Defines a 3D box. */
		Box,
		/* Defines a 3D sphere with 12 divisions. */
		Sphere,
		/* Defines a 3D hemisphere with 12 divisions. */
		Dome
	};

	/* Converts the shape type to a human readable string. */
	_Check_return_ inline const char* to_string(_In_ ShapeType type)
	{
		switch (type)
		{
		case ShapeType::Plane:
			return "Plane";
		case ShapeType::Box:
			return "Box";
		case ShapeType::Sphere:
			return "Sphere";
		case ShapeType::Dome:
			return "Dome";
		default:
			return "Unknown";
		}
	}
}