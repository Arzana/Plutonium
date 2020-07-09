#pragma once
#include "Core/Math/Constants.h"
#include <string>
#include <assert.h>

namespace Pu
{
	/* Defines a vector type that uses SSE vectorization internally. */
	template <typename sse_t, typename single_t, const sse_t lut[8],
	sse_t(*sse_or)(sse_t, sse_t), sse_t(*sse_andnot)(sse_t, sse_t),
	sse_t(*sse_and)(sse_t, sse_t), sse_t(*sse_add)(sse_t, sse_t),
	sse_t(*sse_set1)(single_t)>
	class sse_vector
	{
	public:
		/* Defines the type of this vector. */
		using vector_t = typename sse_vector<sse_t, single_t, lut, sse_or, sse_andnot, sse_and, sse_add, sse_set1>;
		/* Defines an iterator type. */
		using iterator = sse_t*;
		/* Defines a constant iterator type. */
		using const_iterator = const sse_t*;

		/* Initializes an empty instance of an SSE vector. */
		sse_vector(void)
			: buffer(nullptr), cap(0), cnt(0)
		{}

		/* Copy constructor. */
		sse_vector(_In_ const vector_t &value)
			: cap(value.cap), cnt(value.cnt)
		{
			sse_realloc();
			sse_memcpy(value);
		}

		/* Move constructor. */
		sse_vector(_In_ vector_t &&value)
			: buffer(value.buffer), cap(value.cap), cnt(value.cnt)
		{
			value.buffer = nullptr;
			value.cap = 0;
			value.cnt = 0;
		}

		/* Releases the resources allocated by the SSE vector. */
		~sse_vector(void)
		{
			sse_destroy();
		}

		/* Copy assignment. */
		_Check_return_ vector_t& operator =(_In_ const vector_t &other)
		{
			if (this != &other)
			{
				reserve(other.cnt);
				sse_memcpy(other);
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ vector_t& operator =(_In_ vector_t &&other)
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
		_Check_return_ sse_t& operator [](_In_ size_t i)
		{
			return buffer[i];
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ sse_t operator [](_In_ size_t i) const
		{
			return buffer[i];
		}

		/* Returns the iterator pointing to the first element in the vector. */
		_Check_return_ iterator begin(void)
		{
			return buffer;
		}

		/* Returns the iterator pointing to the first element in the vector. */
		_Check_return_ const_iterator begin(void) const
		{
			return buffer;
		}

		/* Returns the iterator referring to the past-the-end element in the vector. */
		_Check_return_ iterator end(void)
		{
			return buffer + sse_cast(cnt);
		}

		/* Returns the iterator referring to the past-the-end element in the vector. */
		_Check_return_ const_iterator end(void) const
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
			return cap << sse_shift;
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
		_Check_return_ sse_t& front(void)
		{
			return *buffer;
		}

		/* Gets the SSE element at the front of the buffer. */
		_Check_return_ sse_t front(void) const
		{
			return *buffer;
		}

		/* Gets the SSE element at the back of the buffer. */
		_Check_return_ sse_t& back(void)
		{
			return buffer[(cnt >> sse_shift) - 1];
		}

		/* Gets the SSE element at the back of the buffer. */
		_Check_return_ sse_t back(void) const
		{
			return buffer[(cnt >> sse_shift) - 1];
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ sse_t& at(_In_ size_t i)
		{
			assert((i >> sse_shift) < cnt && "Index out of range!");
			return buffer[i];
		}

		/* Gets the SSE element at the specified index. */
		_Check_return_ sse_t at(_In_ size_t i) const
		{
			assert((i >> sse_shift) < cnt && "Index out of range!");
			return buffer[i];
		}

		/* Gets the normal type value at the specified index. */
		_Check_return_ single_t get(_In_ size_t idx) const
		{
			assert(idx <= cnt && "Index out of range!");
			const SSE_UNION<sse_t, single_t> helper{ buffer[idx >> sse_shift] };
			return helper.V[idx & sse_mask];
		}

		/* Gets the underlying data buffer. */
		_Check_return_ sse_t* data(void)
		{
			return buffer;
		}

		/* Gets the underlying data buffer. */
		_Check_return_ const sse_t* data(void) const
		{
			return buffer;
		}

		/* Sets the value at the specified index. */
		void set(_In_ size_t idx, _In_ single_t value)
		{
			assert(idx <= cnt && "Index out of range!");

			const size_t i = idx >> sse_shift;
			const size_t j = idx & sse_mask;

			buffer[i] = sse_or(sse_andnot(lut[j], buffer[i]), sse_and(sse_set1(value), lut[j]));
		}

		/* Adds the specified value to the value at the specified index. */
		void add(_In_ size_t idx, _In_ single_t value)
		{
			assert(idx <= cnt && "Index out of range!");

			const size_t i = idx >> sse_shift;
			const size_t j = idx & sse_mask;

			buffer[i] = sse_add(buffer[i], sse_and(sse_set1(value), lut[j]));
		}

		/* Pushes a single value to the vector. */
		void push(_In_ single_t value)
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
		void erase(_In_ size_t idx)
		{
			/* We can just decrease the count and early out if this was the last element. */
			assert(idx < cnt && "Index out of range!");
			if (idx == --cnt) return;

			const size_t i = idx >> sse_shift;
			const size_t j = idx & sse_mask;
			SSE_UNION<sse_t, single_t> shift;

			/*
			This wasn't the last element so we must swift all following element left by one.
			Start at the back and work our way back to the removed element.
			*/
			float carryIn = -NAN, carryOut;
			for (size_t k = cnt >> sse_shift; k > i; k--)
			{
				shift.SSE = buffer[k];
				carryOut = shift.V[0];

				for (size_t l = 0; l < sse_num - 1; l++)
				{
					shift.V[l] = shift.V[l + 1];
				}

				shift.V[sse_num - 1] = carryIn;
				buffer[k] = shift.SSE;
				carryIn = carryOut;
			}

			/*
			Handle the last SSE type differently.
			This is because it doesn't have a carry out.
			*/
			shift.SSE = buffer[i];
			for (size_t k = j; k < sse_num - 1; k++)
			{
				shift.V[k] = shift.V[k + 1];
			}

			/* But it does have a carry in. */
			shift.V[7] = carryIn;
			buffer[i] = shift.SSE;
		}

	private:
		/* 4 for __m128, 8 for __m256. */
		static constexpr size_t sse_num = sizeof(sse_t) / sizeof(single_t);
		/* 3 for __m128, 7 for __m256. */
		static constexpr size_t sse_mask = sse_num - 1ull;
		/* 2 for __m128, 3 for __m256. */
		static constexpr size_t sse_shift = 1ull + sse_num / 4ull;

		sse_t *buffer;
		size_t cap, cnt;

		/* Converts from single count to SSE count (zero becomes zero). */
		static size_t sse_cast(size_t n)
		{
			return (n >> sse_shift) + ((n & sse_mask) != 0);
		}

		void sse_realloc(void)
		{
			buffer = reinterpret_cast<sse_t*>(_aligned_realloc(buffer, cap * sizeof(sse_t), sizeof(sse_t)));
		}

		void sse_memcpy(const vector_t &other)
		{
			memcpy(buffer, other.buffer, cnt * sizeof(sse_t));
		}

		void sse_destroy(void)
		{
			if (buffer) _aligned_free(buffer);
		}
	};

	/* Defines the indexing lookup table for floating point AVX types. */
	extern const ofloat avxf_lut[8];
	/* Defines the indexing lookup table for integer AVX types. */
	extern const int256 avxi_lut[8];
	/* Defines the indexing lookup table for floating point SSE types. */
	extern const qfloat ssef_lut[4];

	/* Creates the name for the sse_vector internal functions. */
#define SSE_VECTOR_PROC_WRAPPER(proc)											sse_vector_proc_wrapper##proc
	/* Creates a static wrapper function for a extern SSE 'set1' function. */
#define SSE_VECTOR_SET1_PROC_WRAPPER(proc, sse, single)							static inline sse SSE_VECTOR_PROC_WRAPPER(proc)(single a) { return proc(a); }
	/* Creates a static wrapper function for an extern SSE function. */
#define SSE_VECTOR_PROC_WRAPPER_CREATE(proc, sse)								static inline sse SSE_VECTOR_PROC_WRAPPER(proc)(sse a, sse b) { return proc(a, b); }
	/* Creates a new SSE vector type and all its underlying SSE wrapper functions. */
#define SSE_VECTOR_CREATE(name, sse, single, lut, or, andnot, and, add, set1)	\
	SSE_VECTOR_SET1_PROC_WRAPPER(set1, sse, single)								\
	SSE_VECTOR_PROC_WRAPPER_CREATE(or, sse)										\
	SSE_VECTOR_PROC_WRAPPER_CREATE(andnot, sse)									\
	SSE_VECTOR_PROC_WRAPPER_CREATE(and, sse)									\
	SSE_VECTOR_PROC_WRAPPER_CREATE(add, sse)									\
	using name = typename sse_vector<sse, single, lut, SSE_VECTOR_PROC_WRAPPER(or), SSE_VECTOR_PROC_WRAPPER(andnot), SSE_VECTOR_PROC_WRAPPER(and), SSE_VECTOR_PROC_WRAPPER(add), SSE_VECTOR_PROC_WRAPPER(set1)>
		
	/* Defines an SSE vector using AVX float internally. */
	SSE_VECTOR_CREATE(avxf_vector, ofloat, float, avxf_lut, _mm256_or_ps, _mm256_andnot_ps, _mm256_and_ps, _mm256_add_ps, _mm256_set1_ps);
	/* Defines an SSE vector using AVX int internally. */
	SSE_VECTOR_CREATE(avxu_vector, int256, uint32, avxi_lut, _mm256_or_si256, _mm256_andnot_si256, _mm256_and_si256, _mm256_add_epi32, _mm256_set1_epi32);
}