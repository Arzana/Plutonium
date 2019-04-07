#pragma once
#include <shared_mutex>
#include "vector.h"

namespace Pu
{
	/* Defines a thread safe pool of a specifed type, this object holds ownership over the objects. */
	template <typename element_t>
	class pool
	{
	public:
		/* Initializes a new instance of a pool. */
		pool(void)
		{}

		/* Copy constructor. */
		pool(_In_ const pool<element_t> &value)
			: buffer(value.buffer)
		{}

		/* Move constructor. */
		pool(_In_ pool<element_t> &&value)
			: lock(std::move(value.lock)), buffer(std::move(value.buffer))
		{}

		/* Copy assignment. */
		_Check_return_ inline pool<element_t>& operator =(_In_ const pool<element_t> &other)
		{
			if (this != &other)
			{
				lock.lock();
				other.lock.lock_shared();
				
				buffer = other.buffer;

				other.lock.unlock_shared();
				lock.unlock();
			}

			return this;
		}

		/* Move assignment. */
		_Check_return_ inline pool<element_t>& operator =(_In_ pool<element_t> &&other)
		{
			if (this != &other)
			{
				lock.lock();
				other.lock.lock();

				buffer = std::move(other.buffer);

				other.lock.unlock();
				lock.unlock();
			}

			return *this;
		}

		/* Adds a new item to the pool. */
		inline void emplace(_In_ element_t&& item)
		{
			lock.lock();
			buffer.emplace_back(std::make_tuple(true, std::move(item)));
			lock.unlock();
		}

		/* Gets and reserves an element from the pool. */
		_Check_return_ inline element_t& get(void)
		{
			lock.lock();

			for (auto &[free, item] : buffer)
			{
				if (free)
				{
					free = false;
					lock.unlock();
					return item;
				}
			}

			ArgOutOfRange();
			lock.unlock();
		}

		/* Frees the element back into the pool. */
		_Check_return_ inline void recycle(_In_ const element_t &item)
		{
			lock.lock();

			for (auto &[free, cur] : buffer)
			{
				if (cur == item)
				{
					free = true;
					lock.unlock();
					return;
				}
			}

			ArgOutOfRange();
			lock.unlock();
		}

		/* Clears the pool. */
		inline void clear(void)
		{
			lock.lock();
			buffer.clear();
			lock.unlock();
		}

		/* Gets the amount of elements in the pool. */
		_Check_return_ inline size_t size(void) const
		{
			lock.lock_shared();
			const size_t result = buffer.size();
			lock.unlock_shared();
			return result;
		}

		/* Gets the amount of free elements. */
		_Check_return_ inline size_t available(void) const
		{
			lock.lock_shared();

			size_t result = 0;
			for (const auto &[free, item] : buffer)
			{
				if (free) result++;
			}

			lock.unlock_shared();
			return result;
		}

	private:
		vector<std::tuple<bool, element_t>> buffer;
		mutable std::shared_mutex lock;

		inline void ArgOutOfRange(void) const
		{
			std::_Xout_of_range("Index was out of range!");
		}
	};
}