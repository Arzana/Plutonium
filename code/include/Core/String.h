#pragma once
#include <string>
#include "Core\Math\Basics.h"
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
	/* Converts bytes to a short string format. */
	_Check_return_ std::string b2short_string(_In_ uint64 value, _In_opt_ uint64 kbBoundry = kb2b(1), _In_opt_ uint64 mbBoundry = mb2b(1), _In_opt_ uint64 gbBoundry = gb2b(1));
}