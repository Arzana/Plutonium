#pragma once
#include "StreamReader.h"

struct _iobuf;

namespace Plutonium
{
	/* Defines an object from reading raw data from file. */
	class FileReader
		: public StreamReader
	{
	public:
		/* Initializes a new file reader from a specified path. */
		FileReader(_In_ const char *path, bool suppressOpen = false);
		/* Initializes a new file reader as a copy. */
		FileReader(_In_ const FileReader &value);
		/* Initializes a new file reader and moves the specified data. */
		FileReader(_In_ FileReader &&value);
		/* Closes the stream and releases the resources of the reader. */
		~FileReader(void) noexcept;

		/* Copies the specified file reader to this file reader. */
		_Check_return_ FileReader& operator =(_In_ const FileReader &other);
		/* Moves the specified file reader to this file reader. */
		_Check_return_ FileReader& operator =(_In_ FileReader &&other);

		/* Gets whether the stream can be used. */
		_Check_return_ inline bool IsOpen(void) const
		{
			return open;
		}

		/* Gets the path of the underlying file. */
		_Check_return_ inline const char* GetFilePath(void) const
		{
			return fpath;
		}

		/* Gets the name of the underlying file (with extension). */
		_Check_return_ inline const char* GetFileName(void) const
		{
			return fname;
		}

		/* Gets the name of the underlying file (without extension). */
		_Check_return_ inline const char* GetFileNameWithoutExtension(void) const
		{
			return fnamenoext;
		}

		/* Gets the type of the underlying file. */
		_Check_return_ inline const char* GetFileExtension(void) const
		{
			return fext;
		}

		/* Gets the directory of the underlying file. */
		_Check_return_ inline const char* GetFileDirectory(void) const
		{
			return fdir;
		}

		/* Closes the underlying stream to the file. */
		void Close(void);
		/*
		Reads the next byte from the file.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Read(void) override;
		/*
		Reads a specified amount of bytes from the file.
		Returns the amount of bytes read.
		*/
		_Check_return_ virtual size_t Read(_Out_ byte *buffer, _In_ size_t offset, _In_ size_t amount) override;
		/*
		Reads untill the end of the current line/
		Requires free!
		*/
		_Check_return_ const char* ReadLine(void);
		/*
		Reads the remainder of the content of the file.
		Requires free!
		*/
		_Check_return_ const char* ReadToEnd(void);
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
		/*
		Seeks the file, increasing it's read position by a sepcified amount.
		*/
		virtual void Seek(_In_ SeekOrigin from, _In_ int64 amount) override;
		/* 
		Gets the current read position of the stream. 
		*/
		_Check_return_ int64 GetPosition(void) const;
		/*
		Gets the total size of the file.
		*/
		_Check_return_ int64 GetSize(void) const;

	private:
		const char *fpath, *fname, *fext, *fnamenoext, *fdir;
		bool open;
		_iobuf *hndlr;

		void SeekInternal(SeekOrigin from, int64 amount) const;
		void Open(void);
		void InitFileArgs(void);
	};
}