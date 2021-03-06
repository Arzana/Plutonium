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
		/* Defines a 3D hemisphere with 24 divisions. */
		Dome,
		/* Defines a 3D revloution of a circle with 24 divisions and a ratio of 0.5. */
		Torus,
		/* Defines a 3D cylinder with 24 divisions. */
		Cylinder,
		/* Defines a 3D cone with 24 divisions. */
		Cone
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
		case ShapeType::Torus:
			return "Torus";
		case ShapeType::Cylinder:
			return "Cylinder";
		case ShapeType::Cone:
			return "Cone";
		default:
			return "Unknown";
		}
	}
}