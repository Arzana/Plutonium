#include "Streams/FileReader.h"
#include "Core/EnumUtils.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Platform/Windows/Windows.h"
#include <cstdio>
#include <cstring>
#include <filesystem>

using namespace Pu;

Pu::FileReader::FileReader(const wstring &path, bool log)
	: fpath(path), open(false), hndlr(nullptr)
{
	Open(log);
}

Pu::FileReader::FileReader(const FileReader & value)
	: fpath(value.fpath), open(false), hndlr(nullptr)
{
	/* Open a new file handle if needed. */
	if (value.open) Open(false);
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
		if (other.open) Open(false);
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

wstring Pu::FileReader::GetCurrentDirectory(void)
{
#ifdef _WIN32
	/* Get raw directory. */
	WCHAR buffer[FILENAME_MAX];
	const DWORD len = WinGetCurrentDirectory(FILENAME_MAX, buffer);

	/* Error check. */
	if (!len)
	{
		const wstring error = _CrtGetErrorString();
		Log::Error("Failed to get working directory (%ls)!", error.c_str());
	}

	/* Return string varient for variable memory release. */
	return wstring(buffer);
#else
	Log::Warning("Cannot get working directory on this platform!");
	return L"";
#endif
}

bool Pu::FileReader::FileExists(const wstring &path)
{
	return std::filesystem::exists(path.c_str());
}

void Pu::FileReader::Close(void)
{
	const wstring fname = fpath.fileName();

	if (open)
	{
		/* Attempt to close the file. */
		if (fclose(hndlr) == EOF) Log::Error("Unable to close file '%ls' (%ls)!", fname.c_str(), FileError().c_str());
		else
		{
			open = false;
			Log::Verbose("Closed file '%ls'.", fname.c_str());
		}
	}
	else Log::Warning("Cannot close non-opened file '%ls'!", fname.c_str());
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
	return fread(buffer + offset, sizeof(byte), amount, hndlr);
}

string Pu::FileReader::ReadLine(void)
{
	/* On debug check if file is open. */
	FileNotOpen();

	/* Read characters until the end of file is reached or a newline character is reached. */
	string result;
	for (int32 c;;)
	{
		c = fgetc(hndlr);

		if (c == EOF || c == '\n') break;

		/* Handle \r\n by just skipping any carriage return in the result but still reading it. */
		if (c != '\r') result += static_cast<char>(c);
	}

	return result;
}

string Pu::FileReader::ReadToEnd(void)
{
	/* On debug check if file is open. */
	FileNotOpen();

	/* Get the remaining length of the file. */
	const int64 pos = GetPosition();
	SeekInternal(SeekOrigin::End, 0);
	const size_t len = static_cast<size_t>(GetPosition());
	SeekInternal(SeekOrigin::Begin, pos);

	/* Allocate space for string and populate it. */
	string result(len);
	const size_t checkLen = fread(result.data(), sizeof(char), len, hndlr);

	/* Check for errors. */
	if (checkLen > len) Log::Fatal("Expected length of string doesn't match actual length!");
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
	if (fseek(hndlr, static_cast<long>(amount), _CrtEnum2Int(from))) Log::Fatal("Unable to seek to position %zd in file '%ls' (%ls)!", amount, fpath.fileName().c_str(), FileError().c_str());
}

void Pu::FileReader::Open(bool log)
{
	const wstring fname = fpath.fileName();

	if (!open)
	{
		/* Attempt to open the file in binary read mode. */
		if (!_wfopen_s(&hndlr, fpath.c_str(), L"rb"))
		{
			open = true;
			Log::Verbose("Successfully opened file '%ls'.", fname.c_str());
		}
		else if (log) Log::Error("Failed to open '%ls' (%ls)!", fpath.c_str(), _CrtGetErrorString().c_str());
	}
	else Log::Warning("Cannot open already opened file '%ls'!", fname.c_str());
}

wstring Pu::FileReader::FileError(void) const
{
	return _CrtFormatError(ferror(hndlr));
}

void Pu::FileReader::FileNotOpen(void)
{
#ifdef _DEBUG
	if (!open) Log::Fatal("File '%ls' isn't open!", fpath.fileName().c_str());
#endif
}