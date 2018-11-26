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
		squeue(_In_ const squeue &value)
			: queue_t(value)
		{}
		/* Move constructor. */
		squeue(_In_ squeue &&value)
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
			EnterReadLock();
			_Ty& elem = queue_t::front();
			LeaveReadLock();
			return elem;
		}

		/* Gets the element at the front of the queue. */
		_Check_return_ inline const _Ty& front(void) const
		{
			EnterReadLock();
			const _Ty& elem = queue_t::front();
			LeaveReadLock();
			return elem;
		}

		/* Gets the element at the back of the queue. */
		_Check_return_ inline _Ty& back(void)
		{
			EnterReadLock();
			_Ty& elem = queue_t::back();
			LeaveReadLock();
			return elem;
		}

		/* Gets the element at the back of the queue. */
		_Check_return_ inline const _Ty& back(void) const
		{
			EnterReadLock();
			const _Ty& elem = queue_t::back();
			LeaveReadLock();
			return elem;
		}

		/* Gets whether the queue is empty. */
		_Check_return_ inline bool empty(void) const
		{
			EnterReadLock();
			const bool isEmpty = queue_t::empty();
			LeaveReadLock();
			return isEmpty;
		}

		/* Gets the amount of elements in the queue. */
		_Check_return_ size_t inline size(void) const
		{
			EnterReadLock();
			const size_t size = queue_t::size();
			LeaveReadLock();
			return size;
		}

		/* Pushes a new element to the back of the queue. */
		void push(_In_ const _Ty &value)
		{
			EnterWriteLock();
			queue_t::push(value);
			LeaveWriteLock();
		}

		/* Pushes a new element to the back of the queue. */
		void push(_In_ _Ty &&value)
		{
			EnterWriteLock();
			queue_t::push(std::move(value));
			LeaveWriteLock();
		}

		/* Pushes a unspecified amount of elements to the back of the queue. */
		template <typename ..._ValTy>
		void push(_In_ _ValTy &&...values)
		{
			EnterWriteLock();
			queue_t::emplace(std::move(values)...);
			LeaveWriteLock();
		}

		/* Removes the element at the front of the queue. */
		void pop(void)
		{
			EnterWriteLock();
			queue_t::pop();
			LeaveWriteLock();
		}

		/* Removes the element at the front of the queue and return it. */
		_Check_return_ _Ty& pop_front(void)
		{
			EnterWriteLock();
			_Ty &elem = queue_t::front();
			queue_t::pop();
			LeaveWriteLock();
			return elem;
		}

	private: 
		mutable std::shared_mutex lock;

		inline void EnterReadLock(void) const
		{
			lock.lock();
		}

		inline void EnterWriteLock(void)
		{
			lock.lock_shared();
		}

		inline void LeaveReadLock(void) const
		{
			lock.unlock();
		}

		inline void LeaveWriteLock(void)
		{
			lock.unlock_shared();
		}
	};
}