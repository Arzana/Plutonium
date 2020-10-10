#pragma once
#include "Core/String.h"
#include "StreamReader.h"

struct _iobuf;

namespace Pu
{
	/* Defines an object for reading raw data from file. */
	class FileReader
		: public StreamReader
	{
	public:
		/* Initializes a new file reader from a specified path (optionally specify whether to log an error if opening the file failed). */
		FileReader(_In_ const wstring &path, _In_opt_ bool log = true);
		/* Copy constructor. */
		FileReader(_In_ const FileReader &value);
		/* Move constructor. */
		FileReader(_In_ FileReader &&value);
		/* Closes the stream and releases the resources of the reader. */
		virtual ~FileReader(void) noexcept override;

		/* Copy assignment. */
		_Check_return_ FileReader& operator =(_In_ const FileReader &other);
		/* Move assignment. */
		_Check_return_ FileReader& operator =(_In_ FileReader &&other);

		/* Get the current (or working) directory. */
		_Check_return_ static wstring GetCurrentDirectory(void);
		/* Checks whether the specified file exists. */
		_Check_return_ static bool FileExists(_In_ const wstring &path);

		/* Gets whether the stream can be used. */
		_Check_return_ inline bool IsOpen(void) const
		{
			return open;
		}

		/* Gets the path of the underlying file. */
		_Check_return_ inline const wstring& GetFilePath(void) const
		{
			return fpath;
		}

		/* Closes the underlying stream to the file. */
		void Close(void);
		/*
		Reads the next byte from the file.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Read(void) override;
		/*
		Attempts to read the specified structure from the file.
		Returns whether the operation was successful, but will advance the stream even if it fails!
		A false result should be considered an error.
		*/
		template <typename struct_t>
		_Check_return_ inline bool Read(_Out_ struct_t &result)
		{
			return Read(reinterpret_cast<byte*>(&result), 0, sizeof(struct_t)) == sizeof(struct_t);
		}
		/*
		Reads a specified amount of bytes from the file.
		Returns the amount of bytes read.
		*/
		_Check_return_ virtual size_t Read(_Out_ byte *buffer, _In_ size_t offset, _In_ size_t amount) override;
		/* Reads untill the end of the current line. */
		_Check_return_ string ReadLine(void);
		/* Reads the remainder of the content of the file. */
		_Check_return_ string ReadToEnd(void);
		/*
		Reads the next byte from the file without increasing the read position.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Peek(void) override;
		/*
		Reads a specified amount of bytes from the file without increasing the read position.
		Returns the actual amount of bytes peeked.
		*/
		_Check_return_ virtual size_t Peek(_Out_ byte *buffer, _In_ size_t offset, _In_ size_t amount) override;
		/* Seeks the file, increasing it's read position by a sepcified amount. */
		virtual void Seek(_In_ SeekOrigin from, _In_ int64 amount) override;
		/* Gets the current read position of the stream. */
		_Check_return_ int64 GetPosition(void) const;
		/* Gets the total size of the file. */
		_Check_return_ int64 GetSize(void) const;
		/* Counts the occurrence of the specified byte in the file. */
		_Check_return_ size_t GetCharacterCount(_In_ char value);

	private:
		wstring fpath;
		bool open;
		_iobuf *hndlr;

		void SeekInternal(SeekOrigin from, int64 amount) const;
		void Open(bool log);
		wstring FileError(void) const;
		void FileNotOpen(void);
	};
}