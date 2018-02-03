#pragma once
#include "Core\Math\Constants.h"
#include "SeekOrigin.h"
#include <sal.h>

namespace Plutonium
{
	/* Defines a base object for reading raw bytes. */
	struct StreamReader
	{
	public:
		StreamReader(_In_ const StreamReader &value) = delete;
		StreamReader(_In_ StreamReader &&value) = delete;

		_Check_return_ StreamReader& operator =(const StreamReader &other) = delete;
		_Check_return_ StreamReader& operator =(StreamReader &&other) = delete;

		/*
		Reads the next byte from the stream.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Read(void) = 0;
		/*
		Reads a specified amount of bytes from the stream.
		Returns the amount of bytes read.
		*/
		_Check_return_ virtual size_t Read(_Out_ char *buffer, _In_ size_t offset, _In_ size_t amount) = 0;
		/*
		Reads the next byte from the stream without increasing the read position.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Peek(void) = 0;
		/*
		Reads a specified amount of bytes from the stream without increasing the read position.
		Returns the actual amount of bytes peeked.
		*/
		_Check_return_ virtual size_t Peek(_Out_ char *buffer, _In_ size_t offset, _In_ size_t amount) = 0;
		/*
		Seeks the stream, increasing it's read position by a sepcified amount.
		*/
		virtual void Seek(_In_ SeekOrigin from, _In_ int64 amount) = 0;

	protected:
		/* Initializes a new instance of a stream reader. */
		StreamReader(void)
		{}
	};
}