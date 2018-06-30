#pragma once
#include <string>
#include "Core\Math\Basics.h"
#include "Core\Math\Vector2.h"
#include "Core\Math\Matrix.h"

namespace Plutonium
{
	/* Converts a pointer to a string form. */
	_Check_return_ std::string to_string(_In_ const void *value);
	/* Converts a floating point value to string form with a specified format. */
	_Check_return_ std::string to_string(_In_ const char *format, float value);
	/* Converts a 2D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector2 value);
	/* Converts a 3D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector3 value);
	/* Converts a 4D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector4 value);
	/* Converts a matrix to a string form. */
	_Check_return_ std::string to_string(_In_ const Matrix &value);
	/* Converts bytes to a KB string format. */
	_Check_return_ std::string b2kb_string(_In_ uint64 value);
	/* Converts bytes to a MB string format. */
	_Check_return_ std::string b2mb_string(_In_ uint64 value);
	/* Converts bytes to a GB string format. */
	_Check_return_ std::string b2gb_string(_In_ uint64 value);
	/* Converts bytes to a short string format. */
	_Check_return_ std::string b2short_string(_In_ uint64 value, _In_opt_ uint64 kbBoundry = 1000, _In_opt_ uint64 mbBoundry = 1000000, _In_opt_ uint64 gbBoundry = 1000000000);
}