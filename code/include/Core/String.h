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
	struct _string
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

		/* Concatinates the specified integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ int32 value)
		{
			_string<_CharTy> buffer;

			if constexpr (sizeof(_CharTy) > sizeof(char))
			{
				char tmp[sizeof(int32) * 8 + 1];
				_itoa_s(value, tmp, 10);
				buffer.underlying = heapwstr(tmp);
			}
			else
			{
				constexpr size_t size = sizeof(int32) * sizeof(_CharTy) * 8 + 1;
				buffer.Allocate(size);
				_itoa_s(value, buffer.underlying, size, 10);
			}

			return operator+=(buffer);
		}

		/* Concatinates the specified unsigned integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ uint32 value)
		{
			_string<_CharTy> buffer;

			if constexpr (sizeof(_CharTy) > sizeof(char))
			{
				char tmp[sizeof(uint32) * 8 + 1];
				_ultoa_s(value, tmp, 10);
				buffer.underlying = heapwstr(tmp);
			}
			else
			{
				constexpr size_t size = sizeof(uint32) * sizeof(_CharTy) * 8 + 1;
				buffer.Allocate(size);
				_ultoa(value, buffer.underlying, size, 10);
			}

			return operator+=(buffer);
		}

		/* Concatinates the specified long integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ int64 value)
		{
			_string<_CharTy> buffer;
			std::string tmp = std::to_string(value);

			if constexpr (sizeof(_CharTy) > sizeof(char)) buffer.underlying = heapwstr(tmp.c_str());
			else buffer.underlying = heaptr(tmp.c_str());

			return operator+=(buffer);
		}

		/* Concatinates the specified unsigned long integer value into this string, returning the result. */
		_Check_return_ _string<_CharTy>& operator +=(_In_ uint64 value)
		{
			_string<_CharTy> buffer;
			std::string tmp = std::to_string(value);

			if constexpr (sizeof(_CharTy) > sizeof(char)) buffer.underlying = heapwstr(tmp);
			else buffer.underlying = heapstr(tmp.c_str());

			return operator+=(buffer);
		}

		/* Gets the length of the string. */
		_Check_return_ inline size_t Length(void) const
		{
			return strlen(underlying);
		}

	private:
		_CharTy * underlying;

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

	/* Converts a pointer to a string form. */
	_Check_return_ std::string to_string(_In_ const void *value);
	/* Converts a floating point value to string form with a specified format. */
	_Check_return_ std::string to_string(_In_ const char *format, float value);
	/* Converts a 2D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector2 value);
	/* Converts a 3D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector3 value);
	/* Converts a 4D vector to a string form. */
	_Check_return_ std::string to_string(_In_ Vector4 value);
	/* Converts a matrix to a string form. */
	_Check_return_ std::string to_string(_In_ const Matrix &value);
	/* Converts bytes to a KB string format. */
	_Check_return_ std::string b2kb_string(_In_ uint64 value);
	/* Converts bytes to a MB string format. */
	_Check_return_ std::string b2mb_string(_In_ uint64 value);
	/* Converts bytes to a GB string format. */
	_Check_return_ std::string b2gb_string(_In_ uint64 value);
	/* Converts bytes to a short string format. */
	_Check_return_ std::string b2short_string(_In_ uint64 value, _In_opt_ uint64 kbBoundry = 1000, _In_opt_ uint64 mbBoundry = 1000000, _In_opt_ uint64 gbBoundry = 1000000000);
}