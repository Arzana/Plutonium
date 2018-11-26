#pragma once
#include "SQueue.h"

namespace Pu
{
	/* Defines a thread safe pool of a specifed type, this object holds ownership over the objects. */
	template <class _Ty, class _Alloc = std::allocator<_Ty>>
	class pool
	{
	public:
		/* Initializes a new instance of a object pool. */
		pool(void)
		{}

		/* Copy contructor. */
		pool(_In_ const pool<_Ty, _Alloc> &value)
			: heap(value.heap), allocator(value.allocator)
		{}

		/* Move constructor. */
		pool(_In_ pool<_Ty, _Alloc> &&value)
			: heap(std::move(value.heap)), allocator(std::move(value.allocator))
		{}

		/* Releases the resources held by the pool. */
		~pool(void)
		{
			collect();
		}

		/* Copy assignment. */
		_Check_return_ pool<_Ty, _Alloc>& operator =(_In_ const pool<_Ty, _Alloc> &other)
		{
			if (this != &other)
			{
				collect();

				heap = other.heap;
				allocator = other.allocator;
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ pool<_Ty, _Alloc>& operator =(_In_ pool<_Ty, _Alloc> &&other)
		{
			if (this != &other)
			{
				collect();

				heap = std::move(other.heap);
				allocator = std::move(other.allocator);
			}

			return *this;
		}

		/* Spawns a new object from the pool. */
		template <class ..._ArgTy>
		_Check_return_ inline _Ty& spawn(_In_opt_ _ArgTy &&...args)
		{
			return heap.empty() ? create(args...) : recycle(args...);
		}

		/* Releases the object back into the pool. */
		void inline kill(_In_ _Ty &obj)
		{
			heap.push(std::addressof(obj));
		}

		/* Clear the pool, releasing all it's resources. */
		void inline collect(void)
		{
			while (!heap.empty())
			{
				ptr_t raw = heap.pop_front();
				alloc_t::deallocate(allocator, raw, 1);
			}
		}

	private:
		using alloc_t = std::allocator_traits<_Alloc>;
		using ptr_t = typename alloc_t::pointer;

		squeue<ptr_t> heap;
		_Alloc allocator;

		template <class ..._ArgsTy>
		_Ty& create(_ArgsTy &&...args)
		{
			ptr_t raw = alloc_t::allocate(allocator, 1);
			alloc_t::construct(allocator, raw, std::move(args)...);
			return *raw;
		}

		template <class ..._ArgsTy>
		_Ty& recycle(_ArgsTy &&...args)
		{
			ptr_t raw = heap.pop_front();
			alloc_t::destroy(allocator, raw);
			*raw = _Ty(args...);
			return *raw;
		}
	};
}