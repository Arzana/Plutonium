#pragma once
#include <queue>
#include <shared_mutex>

namespace Pu
{
	/* Defines a FIFO thread safe queue of generic type. */
	template <class element_t>
	class squeue
		: private std::queue<element_t>
	{
	public:
		using queue_t = typename std::queue<element_t>;

		/* Initializes a new instance of a thread safe queue. */
		squeue(void)
			: queue_t()
		{}
		/* Copy constructor. */
		squeue(_In_ const squeue<element_t> &value)
			: queue_t(value)
		{}
		/* Move constructor. */
		squeue(_In_ squeue<element_t> &&value)
			: queue_t(std::move(value)), lock(std::move(value.lock))
		{}

		/* Copy assignment. */
		_Check_return_ inline squeue<element_t>& operator =(_In_ const squeue<element_t> &other)
		{
			if (&other != this) queue_t::operator=(other);
			return *this;
		}
		/* Move assignment. */
		_Check_return_ inline squeue<element_t>& operator =(_In_ squeue<element_t> &&other)
		{
			if (&other != this) queue_t::operator=(std::move(other));
			return *this;
		}

		/* Gets the element at the front of the queue. */
		_Check_return_ inline element_t& front(void)
		{
			lock.lock_shared();
			element_t& elem = queue_t::front();
			lock.unlock_shared();
			return elem;
		}

		/* Gets the element at the front of the queue. */
		_Check_return_ inline const element_t& front(void) const
		{
			lock.lock_shared();
			const element_t& elem = queue_t::front();
			lock.unlock_shared();
			return elem;
		}

		/* Gets the element at the back of the queue. */
		_Check_return_ inline element_t& back(void)
		{
			lock.lock_shared();
			element_t& elem = queue_t::back();
			lock.unlock_shared();
			return elem;
		}

		/* Gets the element at the back of the queue. */
		_Check_return_ inline const element_t& back(void) const
		{
			lock.lock_shared();
			const element_t& elem = queue_t::back();
			lock.unlock_shared();
			return elem;
		}

		/* Gets whether the queue is empty. */
		_Check_return_ inline bool empty(void) const
		{
			lock.lock_shared();
			const bool isEmpty = queue_t::empty();
			lock.unlock_shared();
			return isEmpty;
		}

		/* Gets the amount of elements in the queue. */
		_Check_return_ size_t inline size(void) const
		{
			lock.lock_shared();
			const size_t size = queue_t::size();
			lock.unlock_shared();
			return size;
		}

		/* Pushes a new element to the back of the queue. */
		void push(_In_ const element_t &value)
		{
			lock.lock();
			queue_t::push(value);
			lock.unlock();
		}

		/* Pushes a new element to the back of the queue. */
		void push(_In_ element_t &&value)
		{
			lock.lock();
			queue_t::push(std::move(value));
			lock.unlock();
		}

		/* Pushes a unspecified amount of elements to the back of the queue. */
		template <typename ..._ValTy>
		void push(_In_ _ValTy &&...values)
		{
			lock.lock();
			queue_t::emplace(std::move(values)...);
			lock.unlock();
		}

		/* Removes the element at the front of the queue. */
		void pop(void)
		{
			lock.lock();
			queue_t::pop();
			lock.unlock();
		}

	private: 
		mutable std::shared_mutex lock;
	};
}