#pragma once
#include <cstring>
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines the types of endians available for the binary reader/writer. */
	enum class Endian
	{
		/* Least significant bytes are first. */
		Little,
		/* Most significant bytes are first. */
		Big
	};

	/* Defines a base class for all stream types. */
	class Stream
	{
	public:
#ifdef _WIN32
		/* Defines the endiant type native to the current platform. */
		constexpr static Endian NativeEndian = Endian::Little;
#else
#error Native endian needs to be set for this platform!
#endif

		Stream(_In_ const Stream&) = delete;
		Stream(_In_ Stream&&) = delete;
		/* Releases the resources allocated by the stream. */
		virtual ~Stream(void)
		{}

		_Check_return_ Stream& operator =(_In_ const Stream&) = delete;
		_Check_return_ Stream& operator =(_In_ Stream&&) = delete;

		/* Gets the magic number for a specified string on compile time. */
		_Check_return_ constexpr static inline int32 GetMagicNum(_In_ const char *str, _In_opt_ int32 start = 0)
		{
			/*
			Loop through string using recursion to allow it to be a constexpr function in C++11.
			For each character add it's value multiplied by it's offset mutiplied by eight to the result.
			*/
			return str[start] != '\0' ? ((str[start] << (start << 3)) + GetMagicNum(str, start + 1)) : 0;
		}

	protected:
		using string_length_t = uint32;

		/* Initializes a new instance of a stream. */
		Stream(void)
		{}

		/* Swaps the bytes per element from its origional endian to the other endian. */
		template <typename raw_t>
		static raw_t ByteSwap(raw_t value, size_t stride = sizeof(raw_t))
		{
			union Cnvrtr
			{
				raw_t normal;
				byte bytes[sizeof(raw_t)];

				Cnvrtr(void) { memset(this, 0, sizeof(Cnvrtr)); }
			} src, dst;

			/* Convert bytes. */
			src.normal = value;
			for (size_t i = 0; i < sizeof(raw_t); i++) dst.bytes[i] = src.bytes[stride * (i / stride + 1) - (i % stride) - 1];
			return dst.normal;
		}
	};
}