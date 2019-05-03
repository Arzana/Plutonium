#include "Streams/FileWriter.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Platform/Windows/Windows.h"
#include <cstdio>
#include <cstring>

using namespace Pu;

Pu::FileWriter::FileWriter(const wstring & path, bool append)
	: fpath(path), created(false), hndl(nullptr)
{
	Create(append ? L"ab" : L"wb");
}

Pu::FileWriter::FileWriter(const FileWriter & value)
	: fpath(value.fpath), created(false), hndl(nullptr)
{
	/* Create a new file handle in append mode if needed. */
	if (value.created) Create(L"ab");
}

Pu::FileWriter::FileWriter(FileWriter && value)
	: fpath(std::move(value.fpath)), created(value.created), hndl(value.hndl)
{
	value.fpath.clear();
	value.created = false;
	value.hndl = nullptr;
}

Pu::FileWriter::~FileWriter(void) noexcept
{
	/* Flushes and closes the file stream if it's still open. */
	if (created)
	{
		Flush();
		Close();
	}
}

FileWriter & Pu::FileWriter::operator=(const FileWriter & other)
{
	if (this != &other)
	{
		/* Closes the stream if it's still open. */
		if (created) Close();

		/* Copy over new data. */
		fpath = other.fpath;

		/* Create a new file handle in append mode if needed. */
		if (other.created) Create(L"ab");
	}

	return *this;
}

FileWriter & Pu::FileWriter::operator=(FileWriter && other)
{
	if (this != &other)
	{
		/*  Closes the stream if it's still open. */
		if (created) Close();

		/* Move file attributes and file handle. */
		fpath = std::move(other.fpath);
		created = other.created;
		hndl = other.hndl;

		/* Clear moved attributes. */
		other.fpath.clear();
		other.created = false;
		other.hndl = nullptr;
	}

	return *this;
}

bool Pu::FileWriter::DirectoryExists(const wstring & directory)
{
#ifdef _WIN32
	const DWORD attrib = GetFileAttributes(directory.c_str());
	return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	Log::Warning("Cannot check if a directoy excists on this platform!");
	return false;
#endif
}

void Pu::FileWriter::CreateDirectory(const wstring & directory)
{
	if (!DirectoryExists(directory))
	{
#ifdef _WIN32
		if (!WinCreateDirectory(directory.c_str(), nullptr)) Log::Error("Unable to create directory (%ls)!", _CrtGetErrorString().c_str());
#else
		Log::Warning("Unable to create directory on this platform!");
#endif
	}
}

void Pu::FileWriter::Close(void)
{
	const wstring fname = fpath.fileName();

	if (created)
	{
		if (fclose(hndl) == EOF) Log::Error("Unable to close file '%ls' (%ls)!", fname.c_str(), FileError().c_str());
		else
		{
			created = false;
			Log::Verbose("Closed file '%ls'!", fname.c_str());
		}
	}
	else Log::Warning("Cannot close non-created file '%ls'!", fname.c_str());
}

void Pu::FileWriter::Flush(void)
{
	FileNotCreated();
	if (fflush(hndl) == EOF) Log::Error("Unable to flush file '%ls' (%ls)!", fpath.fileName().c_str(), FileError().c_str());
}

void Pu::FileWriter::Write(byte value)
{
	if (fwrite(&value, sizeof(byte), 1, hndl) != 1) Log::Error("Unable to write single byte to file '%ls' (%ls)!", fpath.fileName().c_str(), FileError().c_str());
	else if (AutoFlush) Flush();
}

void Pu::FileWriter::Write(const byte * data, size_t offset, size_t amount)
{
	if (fwrite(data + offset, sizeof(byte), amount, hndl) != amount) Log::Error("Unable to write bytes to file '%ls' (%ls)!", fpath.fileName().c_str(), FileError().c_str());
	else if (AutoFlush) Flush();
}

int64 Pu::FileWriter::GetPosition(void) const
{
	return ftell(hndl);
}

void Pu::FileWriter::Create(const wchar_t *mode)
{
	const wstring fname = fpath.fileName();

	if (!created)
	{
		/* Create the file directory if needed. */
		CreateDirectory(fpath.fileDirectory());

		if (!_wfopen_s(&hndl, fpath.c_str(), mode))
		{
			created = true;
			Log::Verbose("Created new handle to file '%ls'.", fname.c_str());
		}
		else Log::Error("Failed to create file '%ls' (%ls)!", fname.c_str(), FileError().c_str());
	}
	else Log::Warning("Cannot create already created file '%ls'!", fname.c_str());
}

wstring Pu::FileWriter::FileError(void) const
{
	return _CrtFormatError(ferror(hndl));
}

void Pu::FileWriter::FileNotCreated(void) const
{
#ifdef _DEBUG
	if (!created) Log::Fatal("File '%ls' wasn't been created!", fpath.fileName().c_str());
#endif
}