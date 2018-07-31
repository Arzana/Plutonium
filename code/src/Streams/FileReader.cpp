#pragma warning(disable:4996)

#include "Streams\FileReader.h"
#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include "Core\EnumUtils.h"
#include <cstdio>
#include <cstring>

using namespace Plutonium;

Plutonium::FileReader::FileReader(const char * path, bool suppressOpen)
	: fpath(heapstr(path)), fname(nullptr), fext(nullptr), fnamenoext(nullptr), fdir(nullptr)
	, open(false), hndlr(nullptr)
{
	/* Initializes the file attributes and open the file. */
	InitFileArgs();
	if (!suppressOpen) Open();
}

Plutonium::FileReader::FileReader(const FileReader & value)
	: open(false), hndlr(nullptr)
{
	/* Copy over the file attributes. */
	fpath = heapstr(value.fpath);
	fname = heapstr(value.fname);
	fext = heapstr(value.fext);
	fnamenoext = heapstr(value.fnamenoext);
	fdir = heapstr(value.fdir);

	/* Open a new file handle if needed. */
	if (value.open) Open();
}

Plutonium::FileReader::FileReader(FileReader && value)
{
	/* Move file attributes and file handle. */
	fpath = value.fpath;
	fname = value.fname;
	fext = value.fext;
	fnamenoext = value.fnamenoext;
	fdir = value.fdir;
	open = value.open;
	hndlr = value.hndlr;

	/* Clear moved attributes. */
	value.fpath = nullptr;
	value.fname = nullptr;
	value.fext = nullptr;
	value.fnamenoext = nullptr;
	value.fdir = nullptr;
	value.open = false;
	value.hndlr = nullptr;
}

Plutonium::FileReader::~FileReader(void) noexcept
{
	/*  Closes the stream if it's still open. */
	if (open) Close();

	/* Release file info. */
	if (fpath) free_s(fpath);
	if (fname) free_s(fname);
	if (fext) free_s(fext);
	if (fnamenoext) free_s(fnamenoext);
	if (fdir) free_s(fdir);
}

FileReader & Plutonium::FileReader::operator=(const FileReader & other)
{
	if (this != &other)
	{
		/*  Closes the stream if it's still open. */
		if (open) Close();

		/* Release file info. */
		if (fpath) free_s(fpath);
		if (fname) free_s(fname);
		if (fext) free_s(fext);
		if (fnamenoext) free_s(fnamenoext);
		if (fdir) free_s(fdir);

		/* Copy over new data. */
		fpath = heapstr(other.fpath);
		fname = heapstr(other.fname);
		fext = heapstr(other.fext);
		fnamenoext = heapstr(other.fnamenoext);
		fdir = heapstr(other.fdir);

		/* Open file is needed. */
		if (other.open) Open();
	}

	return *this;
}

FileReader & Plutonium::FileReader::operator=(FileReader && other)
{
	if (this != &other)
	{
		/*  Closes the stream if it's still open. */
		if (open) Close();

		/* Release file info. */
		if (fpath) free_s(fpath);
		if (fname) free_s(fname);
		if (fext) free_s(fext);
		if (fnamenoext) free_s(fnamenoext);
		if (fdir) free_s(fdir);

		/* Move file attributes and file handle. */
		fpath = other.fpath;
		fname = other.fname;
		fext = other.fext;
		fnamenoext = other.fnamenoext;
		fdir = other.fdir;
		open = other.open;
		hndlr = other.hndlr;

		/* Clear moved attributes. */
		other.fpath = nullptr;
		other.fname = nullptr;
		other.fext = nullptr;
		other.fnamenoext = nullptr;
		other.fdir = nullptr;
		other.open = false;
		other.hndlr = nullptr;
	}

	return *this;
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
	ASSERT_IF(!open, "File isn't open!");
	return fgetc(hndlr);
}

size_t Plutonium::FileReader::Read(byte * buffer, size_t offset, size_t amount)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "File isn't open!");
	return fread(buffer + offset, 1, amount, hndlr);
}

const char * Plutonium::FileReader::ReadLine(void)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "File isn't open!");

	/* Get the current position and set the newline length to a default of one. */
	size_t len = 0, nll = 1;
	int64 pos = GetPosition();

	/* Read characters until some end specifier is found. */
	for (int32 c;; ++len)
	{
		c = fgetc(hndlr);

		/* Newline and end of file can break right away. */
		if (c == '\n' || c == EOF) break;

		/* For carriage return, check if a second control character is used.  */
		if (c == '\r')
		{
			/* Increase newline length if needed. */
			if (Peek() == '\n') ++nll;
			break;
		}
	}

	/* If actual line is empty just return empty string, else seek back to the old position. */
	if (len < 1) return calloc_s(char, 1);
	Seek(SeekOrigin::Begin, pos);

	/* Create and populate result buffer. */
	char *result = malloc_s(char, len + nll);
	size_t checkLen = Read(reinterpret_cast<byte*>(result), 0, len + nll);
	result[len] = '\0';

	/* Check for reading errors. */
	LOG_THROW_IF(checkLen > (len + nll), "Expected length of string doesn't match actual length, this should never occur!");
	return result;
}

const char * Plutonium::FileReader::ReadToEnd(void)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "File isn't open!");

	/* Get the remaining length of the file. */
	int64 pos = GetPosition();
	SeekInternal(SeekOrigin::End, 0);
	int64 len = GetPosition();
	SeekInternal(SeekOrigin::Begin, pos);

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
	/* On debug check if file is open. */
	ASSERT_IF(!open, "File isn't open!");

	/* Get current read position, read char and set read position back. */
	int64 pos = GetPosition();
	int32 result = Read();
	SeekInternal(SeekOrigin::Begin, pos);
	return result;
}

size_t Plutonium::FileReader::Peek(byte * buffer, size_t offset, size_t amount)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "File isn't open!");

	/* Get current read position, read buffer and set read position back. */
	int64 pos = GetPosition();
	size_t result = Read(buffer, offset, amount);
	SeekInternal(SeekOrigin::Begin, pos);
	return result;
}

void Plutonium::FileReader::Seek(SeekOrigin from, int64 amount)
{
	/* On debug check if file is open. */
	ASSERT_IF(!open, "File isn't open!");

	SeekInternal(from, amount);
}

int64 Plutonium::FileReader::GetPosition(void) const
{
	return ftell(hndlr);
}

int64 Plutonium::FileReader::GetSize(void) const
{
	if (!open) return 0;

	int64 oldPos = GetPosition();
	SeekInternal(SeekOrigin::End, 0);

	int64 size = GetPosition();
	SeekInternal(SeekOrigin::Begin, oldPos);
	return size;
}

void Plutonium::FileReader::SeekInternal(SeekOrigin from, int64 amount) const
{
	if (fseek(hndlr, static_cast<long>(amount), _CrtEnum2Int(from))) LOG_THROW("Unable to seek to position %zd in file '%s'!", amount, fname);
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
	/* Count the sub directories in the file path and add one for the file name. */
	constexpr size_t DELIMITER_CNT = 2;
	static char DELIMITERS[DELIMITER_CNT] = { '\\', '/' };
	size_t subDirCnt = cntchar(fpath, DELIMITERS, DELIMITER_CNT);

	/* Split the path in subsections of folders and the file. */
	char **subDirs = mallocaa_s(char, subDirCnt + 1, FILENAME_MAX);
	spltstr(fpath, DELIMITERS, DELIMITER_CNT, subDirs, 0);

	/* Get the file name. */
	fname = heapstr(subDirs[subDirCnt]);

	/* Get the path if it's specified. */
	if (subDirCnt > 0)
	{
		/* Allocate and populate merge buffer. */
		char mrgBuffer[FILENAME_MAX];
		mrgstr(const_cast<const char**>(subDirs), subDirCnt, mrgBuffer, '/');

		/* Add final slash and push buffer to heap. */
		size_t dirLen = strlen(mrgBuffer);
		mrgBuffer[dirLen] = '/';
		mrgBuffer[dirLen + 1] = '\0';
		fdir = heapstr(mrgBuffer);
	}
	else fdir = heapstr("");

	/* Split the name into the file name and the extension. */
	size_t nameAndExtCnt = cntchar(fname, '.');
	char **nameAndExt = mallocaa_s(char, nameAndExtCnt + 1, FILENAME_MAX);
	spltstr(fname, '.', nameAndExt, 0);

	/* Set the file name and extension. */
	if (nameAndExtCnt > 0)
	{
		/* Set file extension. */
		fext = heapstr(nameAndExt[nameAndExtCnt]);

		if (nameAndExtCnt > 1)
		{
			/* Handle case of dots in the file name. */
			fnamenoext = malloc_s(char, strlen(fname) - strlen(fext));
			mrgstr(const_cast<const char**>(nameAndExt), nameAndExtCnt - 1, const_cast<char*>(fnamenoext));
		}
		else
		{
			/* Easy file name just copy values. */
			fnamenoext = heapstr(nameAndExt[0]);
		}
	}
	else
	{
		/* No extension was present so just set it to empty. */
		fnamenoext = heapstr(nameAndExt[0]);
		fext = heapstr("");
	}

	/* Free temporary buffers. */
	freeaa_s(subDirs, subDirCnt + 1);
	freeaa_s(nameAndExt, nameAndExtCnt + 1);
}