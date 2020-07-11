#pragma once
#include "Core/Math/Constants.h"
#include <string>
#include <assert.h>

namespace Pu
{
	/* Defines a vector type that uses SIMD vectorization internally. */
	template <typename simd_t, typename single_t, const simd_t lut[8],
	simd_t(*simd_or)(simd_t, simd_t), simd_t(*simd_andnot)(simd_t, simd_t),
	simd_t(*simd_and)(simd_t, simd_t), simd_t(*simd_add)(simd_t, simd_t),
	simd_t(*simd_set1)(single_t)>
	class simd_vector
	{
	public:
		/* Defines the type of this vector. */
		using vector_t = typename simd_vector<simd_t, single_t, lut, simd_or, simd_andnot, simd_and, simd_add, simd_set1>;
		/* Defines an iterator type. */
		using iterator = simd_t*;
		/* Defines a constant iterator type. */
		using const_iterator = const simd_t*;

		/* Initializes an empty instance of an SIMD vector. */
		simd_vector(void)
			: buffer(nullptr), cap(0), cnt(0)
		{}

		/* Copy constructor. */
		simd_vector(_In_ const vector_t &value)
			: cap(value.cap), cnt(value.cnt)
		{
			simd_realloc();
			simd_memcpy(value);
		}

		/* Move constructor. */
		simd_vector(_In_ vector_t &&value)
			: buffer(value.buffer), cap(value.cap), cnt(value.cnt)
		{
			value.buffer = nullptr;
			value.cap = 0;
			value.cnt = 0;
		}

		/* Releases the resources allocated by the SIMD vector. */
		~simd_vector(void)
		{
			simd_destroy();
		}

		/* Copy assignment. */
		_Check_return_ vector_t& operator =(_In_ const vector_t &other)
		{
			if (this != &other)
			{
				reserve(other.cnt);
				simd_memcpy(other);
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ vector_t& operator =(_In_ vector_t &&other)
		{
			if (this != &other)
			{
				simd_destroy();

				buffer = other.buffer;
				cap = other.cap;
				cnt = other.cnt;

				other.buffer = nullptr;
				other.cap = 0;
				other.cnt = 0;
			}

			return *this;
		}

		/* Gets the SIMD element at the specified index. */
		_Check_return_ simd_t& operator [](_In_ size_t i)
		{
			return buffer[i];
		}

		/* Gets the SIMD element at the specified index. */
		_Check_return_ simd_t operator [](_In_ size_t i) const
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
			return buffer + simd_cast(cnt);
		}

		/* Returns the iterator referring to the past-the-end element in the vector. */
		_Check_return_ const_iterator end(void) const
		{
			return buffer + simd_cast(cnt);
		}

		/* Returns the amount of individual elements in this vector */
		_Check_return_ size_t size(void) const
		{
			return cnt;
		}

		/* Returns the amount of SIMD elements in this vector. */
		_Check_return_ size_t simd_size(void) const
		{
			return simd_cast(cnt);
		}

		/* Returns the amount of individual elements that can fit in this vector. */
		_Check_return_ size_t capacity(void) const
		{
			return cap << simd_shift;
		}

		/* Returns the amount of SIMD elements that can fit in this vector. */
		_Check_return_ size_t simd_capacity(void) const
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
			simd_reserve(simd_cast(amount));
		}

		/* Requests that the vector capacity is at least the specified size. */
		void simd_reserve(_In_ size_t amount)
		{
			if (cap < amount)
			{
				cap = amount;
				simd_realloc();
			}
		}

		/* Requests the vector to deallocate any additional space. */
		void shrink_to_fit(void)
		{
			if (cap > simd_cast(cnt))
			{
				cap = simd_cast(cnt);
				simd_realloc();
			}
		}

		/* Clear the contents of the vector. */
		void clear(void)
		{
			cnt = 0;
		}

		/* Gets the SIMD element at the front of the buffer. */
		_Check_return_ simd_t& front(void)
		{
			return *buffer;
		}

		/* Gets the SIMD element at the front of the buffer. */
		_Check_return_ simd_t front(void) const
		{
			return *buffer;
		}

		/* Gets the SIMD element at the back of the buffer. */
		_Check_return_ simd_t& back(void)
		{
			return buffer[(cnt >> simd_shift) - 1];
		}

		/* Gets the SIMD element at the back of the buffer. */
		_Check_return_ simd_t back(void) const
		{
			return buffer[(cnt >> simd_shift) - 1];
		}

		/* Gets the SIMD element at the specified index. */
		_Check_return_ simd_t& at(_In_ size_t i)
		{
			assert((i >> simd_shift) < cnt && "Index out of range!");
			return buffer[i];
		}

		/* Gets the SIMD element at the specified index. */
		_Check_return_ simd_t at(_In_ size_t i) const
		{
			assert((i >> simd_shift) < cnt && "Index out of range!");
			return buffer[i];
		}

		/* Gets the normal type value at the specified index. */
		_Check_return_ single_t get(_In_ size_t idx) const
		{
			assert(idx <= cnt && "Index out of range!");
			const SIMD_UNION<simd_t, single_t> helper{ buffer[idx >> simd_shift] };
			return helper.V[idx & simd_mask];
		}

		/* Gets the underlying data buffer. */
		_Check_return_ simd_t* data(void)
		{
			return buffer;
		}

		/* Gets the underlying data buffer. */
		_Check_return_ const simd_t* data(void) const
		{
			return buffer;
		}

		/* Sets the value at the specified index. */
		void set(_In_ size_t idx, _In_ single_t value)
		{
			assert(idx <= cnt && "Index out of range!");

			const size_t i = idx >> simd_shift;
			const size_t j = idx & simd_mask;

			buffer[i] = simd_or(simd_andnot(lut[j], buffer[i]), simd_and(simd_set1(value), lut[j]));
		}

		/* Adds the specified value to the value at the specified index. */
		void add(_In_ size_t idx, _In_ single_t value)
		{
			assert(idx <= cnt && "Index out of range!");

			const size_t i = idx >> simd_shift;
			const size_t j = idx & simd_mask;

			buffer[i] = simd_add(buffer[i], simd_and(simd_set1(value), lut[j]));
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
			assert(cnt > 0 && "Cannot pop element from empty SIMD vector!");
			--cnt;
		}

		/* Removes the element at the specified index. */
		void erase(_In_ size_t idx)
		{
			/* We can just decrease the count and early out if this was the last element. */
			assert(idx < cnt && "Index out of range!");
			if (idx == --cnt) return;

			const size_t i = idx >> simd_shift;
			const size_t j = idx & simd_mask;
			SIMD_UNION<simd_t, single_t> shift;

			/*
			This wasn't the last element so we must swift all following element left by one.
			Start at the back and work our way back to the removed element.
			*/
			float carryIn = -NAN, carryOut;
			for (size_t k = cnt >> simd_shift; k > i; k--)
			{
				shift.SIMD = buffer[k];
				carryOut = shift.V[0];

				for (size_t l = 0; l < simd_num - 1; l++)
				{
					shift.V[l] = shift.V[l + 1];
				}

				shift.V[simd_num - 1] = carryIn;
				buffer[k] = shift.SIMD;
				carryIn = carryOut;
			}

			/*
			Handle the last SIMD type differently.
			This is because it doesn't have a carry out.
			*/
			shift.SIMD = buffer[i];
			for (size_t k = j; k < simd_num - 1; k++)
			{
				shift.V[k] = shift.V[k + 1];
			}

			/* But it does have a carry in. */
			shift.V[7] = carryIn;
			buffer[i] = shift.SIMD;
		}

	private:
		/* 4 for __m128, 8 for __m256. */
		static constexpr size_t simd_num = sizeof(simd_t) / sizeof(single_t);
		/* 3 for __m128, 7 for __m256. */
		static constexpr size_t simd_mask = simd_num - 1ull;
		/* 2 for __m128, 3 for __m256. */
		static constexpr size_t simd_shift = 1ull + simd_num / 4ull;

		simd_t *buffer;
		size_t cap, cnt;

		/* Converts from single count to SIMD count (zero becomes zero). */
		static size_t simd_cast(size_t n)
		{
			return (n >> simd_shift) + ((n & simd_mask) != 0);
		}

		void simd_realloc(void)
		{
			buffer = reinterpret_cast<simd_t*>(_aligned_realloc(buffer, cap * sizeof(simd_t), sizeof(simd_t)));
		}

		void simd_memcpy(const vector_t &other)
		{
			memcpy(buffer, other.buffer, cnt * sizeof(simd_t));
		}

		void simd_destroy(void)
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

	/* Creates the name for the simd_vector internal functions. */
#define SIMD_VECTOR_PROC_WRAPPER(proc)											simd_vector_proc_wrapper##proc
	/* Creates a static wrapper function for a extern SIMD 'set1' function. */
#define SIMD_VECTOR_SET1_PROC_WRAPPER(proc, simd, single)						static inline simd SIMD_VECTOR_PROC_WRAPPER(proc)(single a) { return proc(a); }
	/* Creates a static wrapper function for an extern SIMD function. */
#define SIMD_VECTOR_PROC_WRAPPER_CREATE(proc, simd)								static inline simd SIMD_VECTOR_PROC_WRAPPER(proc)(simd a, simd b) { return proc(a, b); }
	/* Creates a new SIMD vector type and all its underlying SIMD wrapper functions. */
#define SIMD_VECTOR_CREATE(name, simd, single, lut, or, andnot, and, add, set1)	\
	SIMD_VECTOR_SET1_PROC_WRAPPER(set1, simd, single)							\
	SIMD_VECTOR_PROC_WRAPPER_CREATE(or, simd)									\
	SIMD_VECTOR_PROC_WRAPPER_CREATE(andnot, simd)								\
	SIMD_VECTOR_PROC_WRAPPER_CREATE(and, simd)									\
	SIMD_VECTOR_PROC_WRAPPER_CREATE(add, simd)									\
	using name = typename simd_vector<simd, single, lut, SIMD_VECTOR_PROC_WRAPPER(or), SIMD_VECTOR_PROC_WRAPPER(andnot), SIMD_VECTOR_PROC_WRAPPER(and), SIMD_VECTOR_PROC_WRAPPER(add), SIMD_VECTOR_PROC_WRAPPER(set1)>
		
	/* Defines an SIMD vector using AVX float internally. */
	SIMD_VECTOR_CREATE(avxf_vector, ofloat, float, avxf_lut, _mm256_or_ps, _mm256_andnot_ps, _mm256_and_ps, _mm256_add_ps, _mm256_set1_ps);
	/* Defines an SIMD vector using AVX int internally. */
	SIMD_VECTOR_CREATE(avxu_vector, int256, uint32, avxi_lut, _mm256_or_si256, _mm256_andnot_si256, _mm256_and_si256, _mm256_add_epi32, _mm256_set1_epi32);
}