#pragma once
#include <deque>
#include <shared_mutex>

namespace Pu
{
	/* Defines a double-ended thread safe queue of generic type. */
	template <class _Ty>
	class sdeque
		: private std::deque<_Ty>
	{
	public:
		using deque_t = typename std::deque<_Ty>;

		/* Initializes a new instance of a thread safe queue. */
		sdeque(void)
			: deque_t()
		{}
		/* Copy constructor. */
		sdeque(_In_ const sdeque<_Ty> &value)
			: deque_t(value)
		{}

		/* Copy assignment. */
		_Check_return_ inline sdeque<_Ty>& operator =(_In_ const sdeque<_Ty> &other)
		{
			if (&other != this) deque_t::operator=(other);
			return *this;
		}

		/* Gets the element at the specified index. */
		_Check_return_ inline _Ty& operator [](_In_ size_t pos)
		{
			lock.lock();
			_Ty &elem = deque_t::operator[](pos);
			lock.unlock();
			return elem;
		}

		/* Gets the element at the specified index. */
		_Check_return_ inline const _Ty& operator [](_In_ size_t pos) const
		{
			lock.lock();
			const _Ty &elem = deque_t::operator[](pos);
			lock.unlock();
			return elem;
		}
		
		/* Gets the element at the specified index. */
		_Check_return_ _Ty& at(_In_ size_t pos)
		{
			lock.lock();
			_Ty &elem = deque_t::at(pos);
			lock.unlock();
			return elem;
		}

		/* Gets the element at the specified index. */
		_Check_return_ const _Ty& at(_In_ size_t pos) const
		{
			lock.lock();
			const _Ty &elem = deque_t::at(pos);
			lock.unlock();
			return elem;
		}

		/* Gets the element at the front of the deque. */
		_Check_return_ _Ty& front(void)
		{
			lock.lock();
			_Ty &elem = deque_t::front();
			lock.unlock();
			return elem;
		}

		/* Gets the element at the front of the deque. */
		_Check_return_ const _Ty& front(void) const
		{
			lock.lock();
			const _Ty &elem = deque_t::front();
			lock.unlock();
			return elem;
		}

		/* Gets the element at the back of the deque. */
		_Check_return_ _Ty& back(void)
		{
			lock.lock();
			_Ty &elem = deque_t::back();
			lock.unlock();
			return elem;
		}

		/* Gets the element at the back of the deque. */
		_Check_return_ const _Ty& back(void) const
		{
			lock.lock();
			const _Ty &elem = deque_t::back();
			lock.unlock();
			return elem;
		}

		/* Gets whether the deque has no elements. */
		_Check_return_ bool empty(void) const noexcept 
		{
			lock.lock();
			const bool isEmpty = deque_t::empty();
			lock.unlock();
			return isEmpty;
		}

		/* Gets the amount of elemetns in this deque. */
		_Check_return_ size_t size(void) const noexcept
		{
			lock.lock();
			const size_t size = deque_t::size();
			lock.unlock();
			return size;
		}

		/* Requests the removal of unused capacity. */
		void shrink_to_fit(void)
		{
			lock.lock_shared();
			deque_t::shrink_to_fit();
			lock.unlock_shared();
		}

		/* Erases all elements from the dequeue. */
		void clear(void) noexcept
		{
			lock.lock_shared();
			deque_t::clear();
			lock.unlock_shared();
		}

		/* Appends the specified item to the end of the deque. */
		void push_back(_In_ const _Ty &value)
		{
			lock.lock_shared();
			deque_t::push_back(value);
			lock.unlock_shared();
		}

		/* Appends the specified item to the end of the deque. */
		void push_back(_In_ _Ty &&value)
		{
			lock.lock_shared();
			deque_t::push_back(std::move(value));
			lock.unlock_shared();
		}

		/* Appends a new element to the end of the deque. */
		template <class ..._ValTy>
		void push_back(_In_ _ValTy &&...args)
		{
			lock.lock_shared();
			deque_t::emplace_back(std::move(args)...);
			lock.unlock_shared();
		}

		/* Removes the last element in the deque and returns it. */
		_Check_return_ _Ty& pop_back(void)
		{
			lock.lock_shared();
			_Ty &elem = deque_t::back();
			deque_t::pop_back();
			lock.unlock_shared();
			return elem;
		}

		/* Attempts to remove the last element in the dequeue and returns it (if the queue is not empty). */
		_Check_return_ bool try_pop_back(_Out_ _Ty &result)
		{
			lock.lock_shared();
			if (!deque_t::empty())
			{
				result = deque_t::back();
				deque_t::pop_back();
				lock.unlock_shared();
				return true;
			}
			
			lock.unlock_shared();
			return false;
		}

		/* Appends the specified item to the front of the deque. */
		void push_front(_In_ const _Ty &value)
		{
			lock.lock_shared();
			deque_t::push_front(value);
			lock.unlock_shared();
		}

		/* Appends the specified item to the front of the deque. */
		void push_front(_In_ _Ty &&value)
		{
			lock.lock_shared();
			deque_t::push_front(std::move(value));
			lock.unlock_shared();
		}

		/* Appends a new element to the front of the deque. */
		template <class ..._ValTy>
		void push_front(_In_ _ValTy &&...args)
		{
			lock.lock_shared();
			deque_t::emplace_front(std::move(args)...);
			lock.unlock_shared();
		}

		/* Removes the first element in the deque and returns it. */
		_Check_return_ _Ty& pop_front(void)
		{
			lock.lock_shared();
			_Ty &elem = deque_t::front();
			deque_t::pop_front();
			lock.unlock_shared();
			return elem;
		}

		/* Attempts to remove the first element in the dequeue and returns it (if the queue is not empty). */
		_Check_return_ bool try_pop_front(_Out_ _Ty &result)
		{
			lock.lock_shared();
			if (!deque_t::empty())
			{
				result = deque_t::front();
				deque_t::pop_front();
				lock.unlock_shared();
				return true;
			}

			lock.unlock_shared();
			return false;
		}

	private:
		mutable std::shared_mutex lock;
	};
}