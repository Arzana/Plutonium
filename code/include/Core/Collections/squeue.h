#pragma once
#include <queue>
#include <shared_mutex>

namespace Pu
{
	/* Defines a FIFO thread safe queue of generic type. */
	template <class _Ty>
	class squeue
		: private std::queue<_Ty>
	{
	public:
		using queue_t = typename std::queue<_Ty>;

		/* Initializes a new instance of a thread safe queue. */
		squeue(void)
			: queue_t()
		{}
		/* Copy constructor. */
		squeue(_In_ const squeue<_Ty> &value)
			: queue_t(value)
		{}
		/* Move constructor. */
		squeue(_In_ squeue<_Ty> &&value)
			: queue_t(std::move(value)), lock(std::move(value.lock))
		{}

		/* Copy assignment. */
		_Check_return_ inline squeue<_Ty>& operator =(_In_ const squeue<_Ty> &other)
		{
			if (&other != this) queue_t::operator=(other);
			return *this;
		}
		/* Move assignment. */
		_Check_return_ inline squeue<_Ty>& operator =(_In_ squeue<_Ty> &&other)
		{
			if (&other != this) queue_t::operator=(std::move(other));
			return *this;
		}

		/* Gets the element at the front of the queue. */
		_Check_return_ inline _Ty& front(void)
		{
			lock.lock_shared();
			_Ty& elem = queue_t::front();
			lock.unlock_shared();
			return elem;
		}

		/* Gets the element at the front of the queue. */
		_Check_return_ inline const _Ty& front(void) const
		{
			lock.lock_shared();
			const _Ty& elem = queue_t::front();
			lock.unlock_shared();
			return elem;
		}

		/* Gets the element at the back of the queue. */
		_Check_return_ inline _Ty& back(void)
		{
			lock.lock_shared();
			_Ty& elem = queue_t::back();
			lock.unlock_shared();
			return elem;
		}

		/* Gets the element at the back of the queue. */
		_Check_return_ inline const _Ty& back(void) const
		{
			lock.lock_shared();
			const _Ty& elem = queue_t::back();
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
		void push(_In_ const _Ty &value)
		{
			lock.lock();
			queue_t::push(value);
			lock.unlock();
		}

		/* Pushes a new element to the back of the queue. */
		void push(_In_ _Ty &&value)
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

		/* Removes the element at the front of the queue and return it. */
		_Check_return_ _Ty& pop_front(void)
		{
			lock.lock();
			_Ty &elem = queue_t::front();
			queue_t::pop();
			lock.unlock();
			return elem;
		}

	private: 
		mutable std::shared_mutex lock;
	};
}