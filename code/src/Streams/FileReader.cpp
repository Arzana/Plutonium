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

size_t Plutonium::FileReader::Read(char * buffer, size_t offset, size_t amount)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "Cannot read byte from file!", "File isn't open!");

	/* Try read from file. */
	if (!fgets(buffer + offset, static_cast<int32>(amount), hndlr))
	{
		/* See if failed because of an error. */
		int err = ferror(hndlr);
		if (err)
		{
			LOG_THROW("Could not read from file, error code: %d!", err);
			return 0;
		}

		/* Check if the end of the file has been reached. */
		if (feof(hndlr))
		{
			LOG_THROW("Framework doesn't allow reading past the buffer size!");
			return 0;
		}

		/* Something went horribly wrong! */
		LOG_THROW("Unknown exception occured whilst reading from file '%s'!", fname);
		return 0;
	}

	/* Handle case for newline. */
	size_t len = strlen(buffer);
	if ((buffer[len - 1] == '\n' || len == 0) && amount > 0)
	{
		size_t result = Read(buffer, len, amount - len + offset);
		return result ? result : len;
	}

	return len;
}

int32 Plutonium::FileReader::Peek(void)
{
	/* Get current read position, read char and set read position back. */
	int64 pos = ftell(hndlr);
	int32 result = Read();
	Seek(SeekOrigin::Begin, pos);
	return result;
}

size_t Plutonium::FileReader::Peek(char * buffer, size_t offset, size_t amount)
{
	/* Get current read position, read buffer and set read position back. */
	int64 pos = ftell(hndlr);
	size_t result = Read(buffer, offset, amount);
	Seek(SeekOrigin::Begin, pos);
	return result;
}

void Plutonium::FileReader::Seek(SeekOrigin from, int64 amount)
{
	if (fseek(hndlr, amount, static_cast<int>(from))) LOG_THROW("Unable to seek to position %zd in file '%s'!", amount, fname);
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
		else LOG_THROW("Failed to open file '%s'!", fname);
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