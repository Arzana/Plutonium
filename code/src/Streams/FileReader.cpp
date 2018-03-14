#pragma warning(disable:4996)

#include "Streams\FileReader.h"
#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include <cstdio>
#include <cstring>

using namespace Plutonium;

Plutonium::FileReader::FileReader(const char * path)
	: fpath(path), fname(""), fext(""), fdir(""), open(false)
{
	/* Initializes the file attributes and open the file. */
	InitFileArgs();
	Open();
}

Plutonium::FileReader::~FileReader(void) noexcept
{
	/*  Closes the stream if it's still open. */
	if (open) Close();
}

void Plutonium::FileReader::Close(void)
{
	if (open)
	{
		/* Attempt to close the file. */
		if (fclose(hndlr) == EOF) LOG_THROW("Unable to close file '%s'!", fname);
		else
		{
			open = false;
			LOG("Closed file '%s'.", fname);
		}
	}
	else LOG_WAR("Cannot close non-opened file '%s'!", fname);
}

int32 Plutonium::FileReader::Read(void)
{
	/* On debug check if file is open, and read from handler. */
	ASSERT_IF(!open, "Cannot read byte from file!", "File isn't open!");
	return fgetc(hndlr);
}

size_t Plutonium::FileReader::Read(byte * buffer, size_t offset, size_t amount)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "Cannot read byte from file!", "File isn't open!");
	return fread(buffer + offset, 1, amount, hndlr);
}

const char * Plutonium::FileReader::ReadToEnd(void)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "Cannot read byte from file!", "File isn't open!");

	/* Get the remaining length of the file. */
	int64 pos = GetPosition();
	Seek(SeekOrigin::End, 0);
	int64 len = GetPosition();
	Seek(SeekOrigin::Begin, pos);

	/* Allocate space for string and populate it. */
	char *result = malloc_s(char, len + 1);
	size_t checkLen = Read(reinterpret_cast<byte*>(result), 0, len);
	result[len] = '\0';

	/* Check for errors. */
	LOG_THROW_IF(static_cast<int64>(checkLen) > len, "Expected length of string doesn't match actual length!");
	return result;
}

int32 Plutonium::FileReader::Peek(void)
{
	/* Get current read position, read char and set read position back. */
	int64 pos = GetPosition();
	int32 result = Read();
	Seek(SeekOrigin::Begin, pos);
	return result;
}

size_t Plutonium::FileReader::Peek(byte * buffer, size_t offset, size_t amount)
{
	/* Get current read position, read buffer and set read position back. */
	int64 pos = GetPosition();
	size_t result = Read(buffer, offset, amount);
	Seek(SeekOrigin::Begin, pos);
	return result;
}

void Plutonium::FileReader::Seek(SeekOrigin from, int64 amount)
{
	if (fseek(hndlr, static_cast<long>(amount), static_cast<int>(from))) LOG_THROW("Unable to seek to position %zd in file '%s'!", amount, fname);
}

int64 Plutonium::FileReader::GetPosition(void) const
{
	return ftell(hndlr);
}

void Plutonium::FileReader::Open(void)
{
	if (!open)
	{
		/* Attempt to open the file in binary read mode. */
		if (hndlr = fopen(fpath, "rb"))
		{
			open = true;
			LOG("Successfully opened file '%s'.", fname);
		}
		else ASSERT("Failed to open file '%s'!", fname);
	}
	else LOG_WAR("Cannot open already opened file '%s'!", fname);
}

void Plutonium::FileReader::InitFileArgs(void)
{
	constexpr size_t DELIM_CNT = 2;
	constexpr size_t BUFFER_LEN = 16;

	/* Split the path in subsections of folders and the file. */
	static char delimiters[DELIM_CNT] = { '\\', '/' };
	char *buffer[BUFFER_LEN];
	for (size_t i = 0; i < BUFFER_LEN; i++) buffer[i] = malloca_s(char, FILENAME_MAX);

	size_t len = spltstr(fpath, delimiters, DELIM_CNT, buffer, 0);

	/* Early out if the attribute split has failed. */
	if (len < 1)
	{
		LOG_WAR("Could not get file attributes from path '%s'!", fpath);
		return;
	}

	/* Get the file name. */
	fname = heapstr(buffer[len - 1]);

	/* Get the file directory. */
	char mrgbuf[FILENAME_MAX];
	mrgstr(buffer, len - 1, mrgbuf, '/');
	len = strlen(mrgbuf);
	mrgbuf[len] = '/';
	mrgbuf[len + 1] = '\0';
	fdir = heapstr(mrgbuf);

	/* Split the name into the file name and extension. */
	len = spltstr(fname, '.', buffer, 0);

	/* Early out if the attributes split has failed. */
	if (len < 1)
	{
		LOG_WAR("Could not get file extension from path '%s'!", fpath);
		return;
	}

	/* Get file extension. */
	fext = heapstr(buffer[len - 1]);
}