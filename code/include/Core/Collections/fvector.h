#pragma once
#include "Core/Math/Constants.h"
#include <string>
#include <assert.h>

namespace Pu
{
	/* Defines a floating point vector type that uses SSE vectorization internally. */
	class fvector
	{
	public:
		/* Defines the underlying SSE type. */
		using sse_type = ofloat;

		/* Initializes an empty instance of a SSE vector. */
		fvector(void)
			: buffer(nullptr), cap(0), cnt(0)
		{}

		/* Copy constructor. */
		fvector(_In_ const fvector &value)
			: cap(value.cap), cnt(value.cnt)
		{
			sse_realloc();
			sse_memcpy(value);
		}

		/* Move constructor. */
		fvector(_In_ fvector &&value)
			: buffer(value.buffer), cap(value.cap), cnt(value.cnt)
		{
			value.buffer = nullptr;
			value.cap = 0;
			value.cnt = 0;
		}

		/* Deallocated the resources allocated by the vector. */
		~fvector(void)
		{
			sse_destroy();
		}

		/* Copy assignment. */
		_Check_return_ fvector& operator =(_In_ const fvector &other)
		{
			if (this != &other)
			{
				reserve(other.cnt);
				sse_memcpy(other);
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ fvector& operator =(_In_ fvector &&other)
		{
			if (this != &other)
			{
				sse_destroy();

				buffer = other.buffer;
				cap = other.cap;
				cnt = other.cnt;

				other.buffer = nullptr;
				other.cap = 0;
				other.cnt = 0;
			}

			return *this;
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ ofloat& operator [](_In_ size_t i)
		{
			return buffer[i];
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ ofloat operator [](_In_ size_t i) const
		{
			return buffer[i];
		}

		/* Returns the iterator pointing to the first element in the vector. */
		_Check_return_ ofloat* begin(void)
		{
			return buffer;
		}

		/* Returns the iterator pointing to the first element in the vector. */
		_Check_return_ const ofloat* begin(void) const
		{
			return buffer;
		}

		/* Returns the iterator referring to the past-the-end element in the vector. */
		_Check_return_ ofloat* end(void)
		{
			return buffer + sse_cast(cnt);
		}

		/* Returns the iterator referring to the past-the-end element in the vector. */
		_Check_return_ const ofloat* end(void) const
		{
			return buffer + sse_cast(cnt);
		}

		/* Returns the amount of individual elements in this vector */
		_Check_return_ size_t size(void) const
		{
			return cnt;
		}

		/* Returns the amount of SSE elements in this vector. */
		_Check_return_ size_t sse_size(void) const
		{
			return sse_cast(cnt);
		}

		/* Returns the amount of individual elements that can fit in this vector. */
		_Check_return_ size_t capacity(void) const
		{
			return cap << 0x3;
		}

		/* Returns the amount of SSE elements that can fit in this vector. */
		_Check_return_ size_t sse_capacity(void) const
		{
			return cap;
		}

		/* Tests whether this vector is empty. */
		_Check_return_ bool empty(void) const
		{
			return !cnt;
		}

		/* Requests that the vector capacity is at least the specified size. */
		void reserve(_In_ size_t amount)
		{
			sse_reserve(sse_cast(amount));
		}

		/* Requests that the vector capacity is at least the specified size. */
		void sse_reserve(_In_ size_t amount)
		{
			if (cap < amount)
			{
				cap = amount;
				sse_realloc();
			}
		}

		/* Requests the vector to deallocate any additional space. */
		void shrink_to_fit(void)
		{
			if (cap > sse_cast(cnt))
			{
				cap = sse_cast(cnt);
				sse_realloc();
			}
		}

		/* Clear the contents of the vector. */
		void clear(void)
		{
			cnt = 0;
		}

		/* Gets the SSE element at the front of the buffer. */
		_Check_return_ ofloat& front(void)
		{
			return *buffer;
		}

		/* Gets the SSE element at the front of the buffer. */
		_Check_return_ ofloat front(void) const
		{
			return *buffer;
		}

		/* Gets the SSE element at the back of the buffer. */
		_Check_return_ ofloat& back(void)
		{
			return buffer[(cnt >> 0x3) - 1];
		}

		/* Gets the SSE element at the back of the buffer. */
		_Check_return_ ofloat back(void) const
		{
			return buffer[(cnt >> 0x3) - 1];
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ ofloat& at(_In_ size_t i)
		{
			assert((i >> 0x3) < cnt && "Index out of range!");
			return buffer[i];
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ ofloat at(_In_ size_t i) const
		{
			assert((i >> 0x3) < cnt && "Index out of range!");
			return buffer[i];
		}

		/* Gets the float at the specified index. */
		_Check_return_ float get(_In_ size_t idx) const
		{
			assert(idx <= cnt && "Index out of range!");
			const AVX_FLOAT_UNION helper{ buffer[idx >> 0x3] };
			return helper.V[idx & 0x7];
		}

		/* Gets the underlying data buffer. */
		_Check_return_ ofloat* data(void)
		{
			return buffer;
		}

		/* Gets the underlying data buffer. */
		_Check_return_ const ofloat* data(void) const
		{
			return buffer;
		}

		/* Sets the value at the specified index. */
		void set(_In_ size_t idx, _In_ float value)
		{
			assert(idx <= cnt && "Index out of range!");

			const size_t i = idx >> 0x3;
			const size_t j = idx & 0x7;

			buffer[i] = _mm256_or_ps(_mm256_andnot_ps(lut[j], buffer[i]), _mm256_and_ps(_mm256_set1_ps(value), lut[j]));
		}

		/* Adds the specified value to the value at the specified index. */
		void add(_In_ size_t idx, _In_ float value)
		{
			assert(idx <= cnt && "Index out of range!");

			const size_t i = idx >> 0x3;
			const size_t j = idx & 0x7;

			buffer[i] = _mm256_add_ps(buffer[i], _mm256_and_ps(_mm256_set1_ps(value), lut[j]));
		}

		/* Pushes a single float to the vector. */
		void push(_In_ float value)
		{
			reserve(cnt + 1);
			set(cnt++, value);
		}

		/* Removes the last element. */
		void pop(void)
		{
			assert(cnt > 0 && "Cannot pop element from empty SSE vector!");
			--cnt;
		}

		/* Removes the element at the specified index. */
		void erase(_In_ size_t idx);

	private:
		static const ofloat lut[8];
		ofloat *buffer;
		size_t cap, cnt;

		static size_t sse_cast(size_t n)
		{
			return (n >> 0x3) + ((n & 0x7) != 0);
		}

		void sse_realloc(void)
		{
			buffer = reinterpret_cast<ofloat*>(_aligned_realloc(buffer, cap * sizeof(ofloat), sizeof(ofloat)));
		}

		void sse_memcpy(const fvector &other)
		{
			memcpy(buffer, other.buffer, cnt * sizeof(ofloat));
		}

		void sse_destroy(void)
		{
			if (buffer) _aligned_free(buffer);
		}
	};
}