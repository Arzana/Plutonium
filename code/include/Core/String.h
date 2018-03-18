#pragma once
#include <string>
#include "Core\Math\Vector2.h"
#include "Core\Math\Matrix.h"

namespace Plutonium
{
	/* Converts a pointer to a string form. */
	_Check_return_ std::string to_string(_In_ const void *value);
	/* Converts a 2D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector2 value);
	/* Converts a 3D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector3 value);
	/* Converts a 4D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector4 value);
	/* Converts a matrix to a string form. */
	_Check_return_ std::string to_string(_In_ const Matrix &value);
}