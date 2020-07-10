#pragma once
#include <assert.h>

namespace Pu
{
	/* Defines a fast stack implementation for non reference types. */
	template <typename element_t>
	class cstack
	{
	public:
		/* Initializes an empty instance of a stack. */
		cstack(void)
			: buffer(nullptr), capacity(0), count(0)
		{}

		/* Initializes an empty instance of a stack with a specified initial capacity. */
		cstack(_In_ size_t initialCapacity)
			: buffer(nullptr), capacity(initialCapacity), count(0)
		{
			Alloc();
		}

		/* Copy constructor. */
		cstack(_In_ const cstack<element_t> &value)
			: buffer(nullptr), capacity(value.capacity), count(value.count)
		{
			CopyAlloc(value.buffer);
		}

		/* Move constructor. */
		cstack(_In_ cstack<element_t> &&value)
			: buffer(value.buffer), count(value.count),
			capacity(value.capacity)
		{
			value.buffer = nullptr;
		}

		/* Releases the resources allocated by the stack. */
		~cstack(void)
		{
			Destroy();
		}

		/* Copy assignment. */
		_Check_return_ inline cstack<element_t>& operator =(_In_ const cstack<element_t> &other)
		{
			if (this != &other)
			{
				capacity = other.capacity;
				count = other.count;
				CopyAlloc(other);
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline cstack<element_t>& operator =(_In_ cstack<element_t> &&other)
		{
			if (this != &other)
			{
				Destroy();

				capacity = other.capacity;
				count = other.count;
				buffer = other.buffer;

				other.buffer = nullptr;
			}

			return *this;
		}

		/* Pushes a new element to the front of the stack. */
		inline void push(_In_ element_t value)
		{
			ensure(count + 1);
			buffer[count++] = value;
		}

		/* Removes and returns the last element. */
		_Check_return_ inline element_t pop(void)
		{
			assert(count > 0 && "Cannot pop a stack with less that one element!");
			return buffer[--count];
		}

		/* Gets the size of the stack. */
		_Check_return_ inline size_t size(void) const
		{
			return count;
		}

		/* Ensures that the stack has the specified amount of space. */
		inline void ensure(_In_ size_t newCapacity)
		{
			if (capacity < newCapacity)
			{
				capacity = newCapacity;
				Alloc();
			}
		}

	private:
		element_t *buffer;
		size_t capacity, count;

		inline void Alloc(void)
		{
			buffer = reinterpret_cast<element_t*>(realloc(buffer, capacity * sizeof(element_t)));
		}

		inline void CopyAlloc(const element_t *other)
		{
			Alloc();
			memcpy(buffer, other, count * sizeof(element_t));
		}

		inline void Destroy(void)
		{
			if (buffer) free(buffer);
		}
	};
}