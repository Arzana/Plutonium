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
	constexpr int BUFFER_SIZE = 256;
	string result;

	/* On debug check if file is open. */
	FileNotOpen();

	/* We use fgets to read until either a newline or EOF. */
	char buffer[BUFFER_SIZE];
	while (fgets(buffer, BUFFER_SIZE, hndlr))
	{
		/*
		If the buffer was too small for the full line,
		fgets will put a null-terminator at the end of the buffer.
		*/
		if (buffer[BUFFER_SIZE - 1] == '\0') result += buffer;
		else
		{
			result = buffer;
			break;
		}
	}

	/* The linefeed is always added at the end, we don't want to return this. */
	return result.trim_back("\r\n");
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

size_t Pu::FileReader::GetCharacterCount(char value)
{
	constexpr size_t BLOCK_SIZE = 4096;
	constexpr size_t AVX_BLOCK_SIZE = BLOCK_SIZE / sizeof(int256);

	/* Just early out if the file wasn't open. */
	if (!open) return 0;
	const int64 oldPos = GetPosition();
	SeekInternal(SeekOrigin::Begin, 0);

	/* Preset these values, loading into an AVX registry is slow. */
#pragma warning (push)
#pragma warning (disable:4309)
	const int256 valueMask = _mm256_set1_epi8(value);
	const int256 one = _mm256_set1_epi8(1);
	const int256 andMask = _mm256_set1_epi8(0x80);
#pragma warning (pop)

	/* Force allignment with an AVX buffer, but also make a byte buffer for ease of use. */
	int256 memory[AVX_BLOCK_SIZE];
	char *bytes = reinterpret_cast<char*>(memory);

	/* Read in block increments (these should be the size of a memory page). */
	size_t result = 0;
	size_t bytesRead;
	while ((bytesRead = fread(bytes, sizeof(char), BLOCK_SIZE, hndlr) > 0))
	{
		/* Set any dangling values to zero, so they won't disturb the count. */
		memset(bytes + bytesRead, 0, BLOCK_SIZE - bytesRead);
		for (size_t i = 0; i < AVX_BLOCK_SIZE; i++)
		{
			/* Use SWAR to check 32 bytes at once. */
			int256 data = _mm256_xor_si256(memory[i], valueMask);
			data = _mm256_and_si256(_mm256_sub_epi8(data, one), _mm256_andnot_si256(data, andMask));
			result += _mm_popcnt_u32(_mm256_movemask_epi8(data));
		}
	}

	SeekInternal(SeekOrigin::Begin, oldPos);
	return result;
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