#pragma warning(disable:4996)

#include "Core\String.h"
#include "Core\SafeMemory.h"
#include "Core\Math\Basics.h"
#include <cstring>
#include <cstdio>

std::string Plutonium::to_string(const void * value)
{
	/* Create buffer to hold converted value. */
	constexpr size_t BUFFER_SIZE = 16;
	char buffer[BUFFER_SIZE];

	/* Convert value to string. */
	snprintf(buffer, BUFFER_SIZE, "%p", value);

	/* Return result as std string. */
	return std::string(buffer);
}

std::string Plutonium::to_string(Vector2 value)
{
	/* Create buffer to hold converted value. */
	constexpr size_t BUFFER_SIZE = 64;
	char buffer[BUFFER_SIZE];

	/* Convert value to string. */
	snprintf(buffer, BUFFER_SIZE, "[X:%f, Y:%f]", value.X, value.Y);

	/* Return result as std string. */
	return std::string(buffer);
}

std::string Plutonium::to_string(Vector3 value)
{
	/* Create buffer to hold converted value. */
	constexpr size_t BUFFER_SIZE = 128;
	char buffer[BUFFER_SIZE];

	/* Convert value to string. */
	snprintf(buffer, BUFFER_SIZE, "[X:%f, Y:%f, Z:%f]", value.X, value.Y, value.Z);

	/* Return result as std string. */
	return std::string(buffer);
}

std::string Plutonium::to_string(Vector4 value)
{
	/* Create buffer to hold converted value. */
	constexpr size_t BUFFER_SIZE = 160;
	char buffer[BUFFER_SIZE];

	/* Convert value to string. */
	snprintf(buffer, BUFFER_SIZE, "[X:%f, Y:%f, Z:%f, W:%f]", value.X, value.Y, value.Z, value.W);

	/* Return result as std string. */
	return std::string(buffer);
}

std::string Plutonium::to_string(const Matrix & value)
{
	/* Create buffer to hold converted value. */
	constexpr size_t BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];

	/* Convert value to string. */
	const float *cmp = value.GetComponents();
	snprintf(buffer, BUFFER_SIZE, "%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f",
			cmp[0], cmp[4], cmp[8], cmp[12],
			cmp[1], cmp[5], cmp[9], cmp[13],
			cmp[2], cmp[6], cmp[10], cmp[14],
			cmp[3], cmp[7], cmp[11], cmp[15]);

	/* Return result as std string. */
	return std::string(buffer);
}

std::string Plutonium::b2short_string(uint64 value, uint64 kbBoundry, uint64 mbBoundry, uint64 gbBoundry)
{
	/* Converts the bytes to the needed level and returns it as a string. */
	if (value < kbBoundry) return std::to_string(value).append(" B");
	else if (value < mbBoundry) return std::to_string(b2kb(value)).append(" KB");
	else if (value < gbBoundry) return std::to_string(b2mb(value)).append(" MB");
	else return std::to_string(b2gb(value)).append(" GB");
}