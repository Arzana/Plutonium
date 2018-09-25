#pragma once
#include <string>
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Core\Math\Basics.h"
#include "Core\Math\Vector2.h"
#include "Core\Math\Matrix.h"

namespace Plutonium
{
	template <typename _CharTy>
	class _string
	{
	public:
		/* Initializes an empty instance of a string. */
		_string(void)
			: underlying(nullptr)
		{}

		/* Initializes a new instance of a string as a copy of a c style string. */
		_string(_In_ const _CharTy *src)
			: _string(src, 0, strlen(src))
		{}

		/* Initializes a new instance of a string as a copy of a subtring of a c style string. */
		_string(_In_ const _CharTy *src, _In_ size_t start, _In_ size_t length)
		{
			Allocate(length + 1);
			Concat(src + start, 0, length);
			AddNullTerminator(length);
		}

		/* Initializes a new instance of a string as a specified character repeated for a specified amount. */
		_string(_In_ _CharTy c, _In_ size_t count)
		{
			Allocate(count + 1);
			for (size_t i = 0; i < count; i++) underlying[i] = c;
			AddNullTerminator(count);
		}

		/* Initializes a new instance of a string as a copy of another string. */
		_string(_In_ const _string<_CharTy> &value)
			: _string(value.underlying)
		{}

		/* Initializes a new instance of a string with the values of another string. */
		_string(_In_ _string<_CharTy> &&value)
			: underlying(value.underlying)
		{
			value.underlying = nullptr;
		}

		/* Releases the resources allocated by the string. */
		~_string(void) noexcept
		{
			Dispose();
		}

		/* Sets the value of this string to a copy of another string. */
		_Check_return_ _string<_CharTy>& operator =(_In_ const _string<_CharTy> &other)
		{
			if (&other != this)
			{
				Dispose();

				const size_t len = other.Length();
				Allocate(len + 1);
				Concat(other.underlying, 0, len);
				AddNullTerminator(len);
			}

			return *this;
		}

		/* Moves the values of the specified string into this string. */
		_Check_return_ _string<_CharTy>& operator =(_In_ _string<_CharTy> &&other)
		{
			if (&other != this)
			{
				underlying = other.underlying;
				other.underlying = nullptr;
			}

			return *this;
		}

		/* Gets the underlying c-style string. */
		_Check_return_ operator const _CharTy*(void) const
		{
			return underlying;
		}

		/* Concatinates the specified string into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ const _string<_CharTy> &value)
		{
			const size_t tLen = Length();
			const size_t rLen = tLen + value.Length();

			realloc_s(_CharTy, underlying, rLen + 1);
			Concat(value.underlying, tLen, rLen);
			AddNullTerminator(rLen);

			return *this;
		}
		/* Concatinates the specified string into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ const _CharTy *value)
		{
			return operator+=(_string<_CharTy>(value));
		}

		/* Concatinates a single specified character to the string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ _CharTy value)
		{
			const size_t tLen = Length();

			realloc_s(_CharTy, underlying, tLen + 2);
			underlying[tLen] = value;
			AddNullTerminator(tLen + 1);

			return *this;
		}

		/* Concatinates the specified integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ int32 value)
		{
			return operator+=(Format<12>("%d", value));
		}

		/* Concatinates the specified unsigned integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ uint32 value)
		{
			return operator+=(Format<12>("%u", value));
		}

		/* Concatinates the specified long integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ int64 value)
		{
			return operator+=(Format<21>("%lld", value));
		}

		/* Concatinates the specified unsigned long integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ uint64 value)
		{
			return operator+=(Format<21>("%llu", value));
		}

		/* Concatinates the specified floating point value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ float value)
		{
			return operator+=(Format<32>("%f", value));
		}

		/* Concatinates the specified pointer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ const void *value)
		{
			return operator+=(Format<19>("0x%p", value));
		}

		/* Concatinates the specified vector value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ Vector2 value)
		{
			return operator+=(Format<64>("[X:%f, Y:%f]", value.X, value.Y));
		}

		/* Concatinates the specified vector value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ Vector3 value)
		{
			return operator+=(Format<128>("[X:%f, Y:%f, Z:%f]", value.X, value.Y, value.Z));
		}

		/* Concatinates the specified vector value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ Vector4 value)
		{
			return operator+=(Format<160>("[X:%f, Y:%f, Z:%f, W:%f]", value.X, value.Y, value.Z, value.W));
		}

		/* Concatinates the specified matrix value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ const Matrix &value)
		{
			const float *f = value.GetComponents();
			return operator+=(Format<512>("%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f",
				f[0], f[4], f[8], f[12],
				f[1], f[5], f[9], f[13],
				f[2], f[6], f[10], f[14],
				f[3], f[7], f[11], f[15]));
		}

		/* Concatinates the specified float with a specified format into this string, returning the result. */
		_Check_return_ _string<_CharTy>& AddFormattedFloat(_In_ const char *format, _In_ float value)
		{
			return operator+=(Format<32>(format, value));
		}

		/* Converts the specified amount of bytes to kilobytes and concatinates them into this string, returning the result. */
		_Check_return_ _string<_CharTy>& AddKiloBytes(_In_ uint64 bytes)
		{
			operator+=(b2kb(bytes));
			if constexpr (sizeof(_CharTy) > sizeof(char)) return operator+=(U" KB");
			else return operator+=(" KB");
		}

		/* Converts the specified amount of bytes to megabytes and concatinates them into this string, returning the result. */
		_Check_return_ _string<_CharTy>& AddMegaBytes(_In_ uint64 bytes)
		{
			operator+=(b2mb(bytes));
			if constexpr (sizeof(_CharTy) > sizeof(char)) return operator+=(U" MB");
			else return operator+=(" MB");
		}

		/* Converts the specified amount of bytes to gigabytes and concatinates them into this string, returning the result. */
		_Check_return_ _string<_CharTy>& AddGigaBytes(_In_ uint64 bytes)
		{
			operator+=(b2gb(bytes));
			if constexpr (sizeof(_CharTy) > sizeof(char)) return operator+=(U" GB");
			else return operator+=(" GB");
		}

		/* Converts the specified amount of bytes to a short string version and concatinates them into this string, returning the result. */
		_Check_return_ _string<_CharTy>& AddShortBytes(_In_ uint64 bytes, _In_opt_ uint64 kbBoundry = 1000, _In_opt_ uint64 mbBoundry = 1000000, _In_opt_ uint64 gbBoundry = 1000000000)
		{
			if (bytes <= kbBoundry)
			{
				operator+=(bytes);
				if constexpr (sizeof(_CharTy) > sizeof(char)) return operator+=(U" B");
				else return operator+=(" B");
			}
			else if (bytes <= mbBoundry) return AddKiloBytes(bytes);
			else if (bytes <= gbBoundry) return AddMegaBytes(bytes);
			else return AddGigaBytes(bytes);
		}

		/* Gets the length of the string. */
		_Check_return_ inline size_t Length(void) const
		{
			return strlen(underlying);
		}

	private:
		_CharTy * underlying;

		template <size_t _BufferSize, typename ... _ArgTy>
		static inline _string<_CharTy> Format(const char *format, _ArgTy ... args)
		{
			char buffer[_BufferSize];
			_string<_CharTy> result;

			sprintf_s(buffer, format, args...);
			if constexpr (sizeof(_CharTy) > sizeof(char)) result.underlying = heapwstr(buffer);
			else result.underlying = heapstr(buffer);

			return result;
		}

		inline void Allocate(size_t size)
		{
			underlying = malloc_s(_CharTy, size);
		}

		inline void Concat(const _CharTy *src, size_t start, size_t end)
		{
			for (size_t i = start, j = 0; i < end; i++, j++) underlying[i] = src[j];
		}

		inline void AddNullTerminator(size_t at)
		{
			underlying[at] = static_cast<_CharTy>(0);
		}

		inline void Dispose(void)
		{
			if (underlying) free_s(underlying);
		}
	};

	/* Defines a basic ASCII string. */
	using string = _string<char>;
	/* Defines a UTF-32 string. */
	using ustring = _string<char32_t>;
}