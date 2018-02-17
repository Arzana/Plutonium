#pragma warning(disable:4996)

#include "Core\String.h"
#include "Core\SafeMemory.h"
#include <cstring>
#include <cstdio>

using namespace Plutonium;

Plutonium::String::String(void)
	:str(""), len(0)
{}

Plutonium::String::String(const char * value)
	: str(const_cast<char*>(value)), len(strlen(value))
{}

Plutonium::String::~String(void)
{
	if (str != "") free_s(str);
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
	constexpr size_t BUFF_SIZE = 16;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "%p", value);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(int value)
{
	constexpr size_t BUFF_SIZE = 32;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "%d", value);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(float value)
{
	constexpr size_t BUFF_SIZE = 32;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "%f", value);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(Vector2 value)
{
	constexpr size_t BUFF_SIZE = 64;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "[X:%f, Y:%f]", value.X, value.Y);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(Vector3 value)
{
	constexpr size_t BUFF_SIZE = 128;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "[X:%f, Y:%f, Z:%f]", value.X, value.Y, value.Z);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(Vector4 value)
{
	constexpr size_t BUFF_SIZE = 160;
	char *buffer = malloca_s(char, BUFF_SIZE);
	snprintf(buffer, BUFF_SIZE, "[X:%f, Y:%f, Z:%f, W:%f]", value.X, value.Y, value.Z, value.W);
	MrgInto(buffer);
	return *this;
}

String & Plutonium::String::operator+=(const Matrix & value)
{
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
	str = "";
	len = 0;
}

void Plutonium::String::MrgInto(const char * value)
{
	const size_t vlen = strlen(value);
	char *buffer = malloc_s(char, len + vlen + 1);
	strncpy(buffer, str, len);
	strncpy(buffer + len, value, vlen);
	len += vlen;
	buffer[len] = '\0';
	str = buffer;
}