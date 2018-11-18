#include "Streams/FileReader.h"
#include "Streams/FileUtils.h"
#include "Core/EnumUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Platform/Windows/Windows.h"
#include <cstdio>
#include <cstring>

using namespace Pu;

Pu::FileReader::FileReader(const char * path)
	: fpath(path), open(false), hndlr(nullptr)
{
	Open();
}

Pu::FileReader::FileReader(const FileReader & value)
	: fpath(value.fpath), open(false), hndlr(nullptr)
{
	/* Open a new file handle if needed. */
	if (value.open) Open();
}

Pu::FileReader::FileReader(FileReader && value)
	: fpath(std::move(value.fpath)), open(value.open), hndlr(value.hndlr)
{
	/* Clear moved attributes. */
	value.fpath.clear();
	value.open = false;
	value.hndlr = nullptr;
}

Pu::FileReader::~FileReader(void) noexcept
{
	/*  Closes the stream if it's still open. */
	if (open) Close();
}

FileReader & Pu::FileReader::operator=(const FileReader & other)
{
	if (this != &other)
	{
		/*  Closes the stream if it's still open. */
		if (open) Close();

		/* Copy over new data. */
		fpath = other.fpath;

		/* Open file is needed. */
		if (other.open) Open();
	}

	return *this;
}

FileReader & Pu::FileReader::operator=(FileReader && other)
{
	if (this != &other)
	{
		/*  Closes the stream if it's still open. */
		if (open) Close();

		/* Move file attributes and file handle. */
		fpath = std::move(other.fpath);
		open = other.open;
		hndlr = other.hndlr;

		/* Clear moved attributes. */
		other.fpath.clear();
		other.open = false;
		other.hndlr = nullptr;
	}

	return *this;
}

string Pu::FileReader::GetCurrentDirectory(void)
{
#ifdef _WIN32
	/* Get raw directory. */
	TCHAR buffer[FILENAME_MAX];
	const DWORD len = WinGetCurrentDirectory(FILENAME_MAX, buffer);

	/* Error check. */
	if (!len)
	{
		const string error = _CrtGetErrorString();
		Log::Error("Failed to get working directory (%s)!", error.c_str());
	}

	/* Return string varient for variable memory release. */
	return string(buffer);
#else
	Log::Warning("Cannot get working directory on this platform!");
	return "";
#endif
}

void Pu::FileReader::Close(void)
{
	const string fname = _CrtGetFileName(fpath);

	if (open)
	{
		/* Attempt to close the file. */
		if (fclose(hndlr) == EOF) Log::Error("Unable to close file '%s' (%s)!", fname.c_str(), FileError().c_str());
		else
		{
			open = false;
			Log::Verbose("Closed file '%s'.", fname.c_str());
		}
	}
	else Log::Warning("Cannot close non-opened file '%s'!", fname.c_str());
}

int32 Pu::FileReader::Read(void)
{
	/* On debug check if file is open, and read from handler. */
	FileNotOpen();
	return fgetc(hndlr);
}

size_t Pu::FileReader::Read(byte * buffer, size_t offset, size_t amount)
{
	/* On debug check if file is open. */
	FileNotOpen();
	return fread(buffer + offset, 1, amount, hndlr);
}

string Pu::FileReader::ReadLine(void)
{
	/* On debug check if file is open. */
	FileNotOpen();

	/* Get the current position and set the newline length to a default of one. */
	size_t len = 0, nll = 1;
	const int64 pos = GetPosition();

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
	if (len < 1) return string();
	Seek(SeekOrigin::Begin, pos);

	/* Create and populate result buffer. */
	string result(len + nll, ' ');
	const size_t checkLen = Read(reinterpret_cast<byte*>(&result[0]), 0, len + nll);
	result[len] = '\0';

	/* Check for reading errors. */
	if (checkLen > (len + nll)) Log::Fatal("Expected length of string doesn't match actual length, this should never occur!");
	return result;
}

string Pu::FileReader::ReadToEnd(void)
{
	/* On debug check if file is open. */
	FileNotOpen();

	/* Get the remaining length of the file. */
	const int64 pos = GetPosition();
	SeekInternal(SeekOrigin::End, 0);
	const int64 len = GetPosition();
	SeekInternal(SeekOrigin::Begin, pos);

	/* Allocate space for string and populate it. */
	string result(len + 1, ' ');
	const size_t checkLen = Read(reinterpret_cast<byte*>(&result[0]), 0, len);
	result[len] = '\0';

	/* Check for errors. */
	if (static_cast<int64>(checkLen) > len) Log::Fatal("Expected length of string doesn't match actual length!");
	return result;
}

int32 Pu::FileReader::Peek(void)
{
	/* On debug check if file is open. */
	FileNotOpen();

	/* Get current read position, read char and set read position back. */
	const int64 pos = GetPosition();
	const int32 result = Read();
	SeekInternal(SeekOrigin::Begin, pos);
	return result;
}

size_t Pu::FileReader::Peek(byte * buffer, size_t offset, size_t amount)
{
	/* On debug check if file is open. */
	FileNotOpen();

	/* Get current read position, read buffer and set read position back. */
	const int64 pos = GetPosition();
	const size_t result = Read(buffer, offset, amount);
	SeekInternal(SeekOrigin::Begin, pos);
	return result;
}

void Pu::FileReader::Seek(SeekOrigin from, int64 amount)
{
	/* On debug check if file is open. */
	FileNotOpen();
	SeekInternal(from, amount);
}

int64 Pu::FileReader::GetPosition(void) const
{
	return ftell(hndlr);
}

int64 Pu::FileReader::GetSize(void) const
{
	if (!open) return 0;

	const int64 oldPos = GetPosition();
	SeekInternal(SeekOrigin::End, 0);

	const int64 size = GetPosition();
	SeekInternal(SeekOrigin::Begin, oldPos);
	return size;
}

void Pu::FileReader::SeekInternal(SeekOrigin from, int64 amount) const
{
	if (fseek(hndlr, static_cast<long>(amount), _CrtEnum2Int(from))) Log::Fatal("Unable to seek to position %zd in file '%s' (%s)!", amount, _CrtGetFileName(fpath).c_str(), FileError().c_str());
}

void Pu::FileReader::Open(void)
{
	const string fname = _CrtGetFileName(fpath);

	if (!open)
	{
		/* Attempt to open the file in binary read mode. */
		if (!fopen_s(&hndlr, fpath.c_str(), "rb"))
		{
			open = true;
			Log::Verbose("Successfully opened file '%s'.", fname.c_str());
		}
		else Log::Error("Failed to open file '%s' (%s)!", fname.c_str(), FileError().c_str());
	}
	else Log::Warning("Cannot open already opened file '%s'!", fname.c_str());
}

string Pu::FileReader::FileError(void) const
{
	return _CrtFormatError(ferror(hndlr));
}

void Pu::FileReader::FileNotOpen(void)
{
#ifdef _DEBUG
	if (!open) Log::Fatal("File '%s' isn't open!", _CrtGetFileName(fpath).c_str());
#endif
}