#pragma once
#include "Core/String.h"
#include "StreamWriter.h"
#include "Core/Collections/Vector.h"

struct _iobuf;

namespace Pu
{
	/* Defines an object for writing raw data to file. */
	class FileWriter
		: public StreamWriter
	{
	public:
		/* Initializes a new file writer from a specified path. */
		FileWriter(_In_ const wstring &path, _In_opt_ bool append = false);
		/* Copy constructor. */
		FileWriter(_In_ const FileWriter &value);
		/* Copy assignment. */
		FileWriter(_In_ FileWriter &&value);
		/* Closes the stream and releases the resources of the writer. */
		virtual ~FileWriter(void) override;

		/* Copy assignment. */
		_Check_return_ FileWriter& operator =(_In_ const FileWriter &other);
		/* Move assignment. */
		_Check_return_ FileWriter& operator =(_In_ FileWriter &&other);

		/* Checks if the specified directory exists. */
		_Check_return_ static bool DirectoryExists(_In_ const wstring &directory);
		/* Creates a directory if it doesn't exist yet. */
		static void CreateDirectory(_In_ const wstring &directory);

		/* Gets whether the stream can be used. */
		_Check_return_ inline bool IsCreated(void) const
		{
			return created;
		}

		/* Gets the path of the underlying file. */
		_Check_return_ inline const wstring& GetFilePath(void) const
		{
			return fpath;
		}

		/* Closes the underlying stream to the file. */
		void Close(void);
		/* Flushes the stream, writing the data into the underlying file. */
		virtual void Flush(void) override;
		/* Writes a single byte to the file. */
		virtual void Write(_In_ byte value) override;
		/* Writes a specific range of bytes to the stream. */
		virtual void Write(_In_ const byte *data, _In_ size_t offset, _In_ size_t amount) override;
		/* Gets the current write position of the stream. */
		_Check_return_ int64 GetPosition(void) const;

	private:
		wstring fpath;
		bool created;
		_iobuf *hndl;

		void Create(const wchar_t *mode);
		wstring FileError(void) const;
		void FileNotCreated(void) const;
	};
}