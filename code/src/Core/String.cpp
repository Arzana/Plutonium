#pragma warning(disable:4996)

#include "Core\String.h"
#include "Core\SafeMemory.h"
#include <cstring>
#include <cstdio>

using namespace Plutonium;

Plutonium::String::String(void)
	: str(""), len(0), heap(false)
{}

Plutonium::String::String(const char * value)
	: str(const_cast<char*>(value)), len(strlen(value)), heap(false)
{}

Plutonium::String::String(String && value)
{
	/* Make sure we don't move to ourselves. */
	if (&value != this)
	{
		/* Copy data to this string. */
		str = value.str;
		len = value.len;
		heap = value.heap;

		/* Set values to default in old string. */
		value.str = "";
		value.len = 0;
		value.heap = false;
	}
}

Plutonium::String::~String(void)
{
	/* Make sure the underlying string is freed is needed. */
	Clear();
}

String & Plutonium::String::operator=(String && other)
{
	/* Make sure we don't move to ourselves. */
	if (&other != this)
	{
		/* Copy data to this string. */
		str = other.str;
		len = other.len;
		heap = other.heap;

		/* Set values to default in old string. */
		other.str = "";
		other.len = 0;
		other.heap = false;
	}

	return *this;
}

String Plutonium::String::operator=(const char * value)
{
	return String(value);
}

Plutonium::String::operator const char*(void) const
{
	return str;
}

String & Plutonium::String::operator+=(const String & other)
{
	MrgInto(other.str);
	return *this;
}

String & Plutonium::String::operator+=(const char * value)
{
	MrgInto(value);
	return *this;
}

String & Plutonium::String::operator+=(const void * value)
{
	/* Convert pointer to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 16;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "%p", value);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(int value)
{
	/* Convert integer to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 32;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "%d", value);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(float value)
{
	/* Convert floating point to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 32;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "%f", value);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(Vector2 value)
{
	/* Convert 2D vector to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 64;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "[X:%f, Y:%f]", value.X, value.Y);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(Vector3 value)
{
	/* Convert 3D vector to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 128;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "[X:%f, Y:%f, Z:%f]", value.X, value.Y, value.Z);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(Vector4 value)
{
	/* Convert 4D vector to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 160;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "[X:%f, Y:%f, Z:%f, W:%f]", value.X, value.Y, value.Z, value.W);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(const Matrix & value)
{
	/* Convert matrix to string and append it to the current string. */
	constexpr size_t BUFF_SIZE = 512;
	char *buffer = malloca_s(char, BUFF_SIZE);
	const float *cmp = value.GetComponents();
	snprintf(buffer, BUFF_SIZE, "%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f",
			 cmp[0], cmp[4], cmp[8], cmp[12],
			 cmp[1], cmp[5], cmp[9], cmp[13],
			 cmp[2], cmp[6], cmp[10], cmp[14],
			 cmp[3], cmp[7], cmp[11], cmp[15]);
	MrgInto(buffer);
	return *this;
}

void Plutonium::String::Clear(void)
{
	/* Make sure the memory is freed is needed. */
	if (heap)
	{
		free_s(str);
		heap = false;
	}

	/* Set values back to default state. */
	str = "";
	len = 0;
}

void Plutonium::String::MrgInto(const char * value)
{
	/* Get the actual length of the value. */
	const size_t vlen = strlen(value);

	/* If the current string is already on the heap simple reallocate more space. */
	if (heap) str = realloc_s(char, str, len + vlen + 1);
	else
	{
		/* If the current string is not on the heap create a new buffer that is on the heap. */
		heap = true;
		char *buffer = malloc_s(char, len + vlen + 1);
		strncpy(buffer, str, len);
		str = buffer;
	}

	/* Copy the value into the defined buffer. */
	strncpy(str + len, value, vlen);

	/* Update the strings length and add a null terminator. */
	len += vlen;
	str[len] = '\0';
}