#pragma once
#include <vector>

namespace Pu
{
	/* Defines a wrapper object for ease of use vector functions. */
	template <class _Ty>
	class vector
		: public std::vector<_Ty>
	{
	public:
		using vector_t = typename std::vector<_Ty>;
		using allocator_t = typename vector_t::allocator_type;
		using const_iterator = typename vector_t::const_iterator;

		/* Initializes an empty instance of a Plutonium vector. */
		vector(void) noexcept(noexcept(allocator_t()))
			: vector_t()
		{}

		/* Initializes a new instance of a Plutonium vector with a specified allocator. */
		explicit vector(_In_ const allocator_t& alloc) noexcept
			: vector_t(alloc)
		{}

		/* Initializes a new instance of a Plutonium vector with a specified amount of copies of the specified value. */
		vector(_In_ size_t count, _In_ const _Ty &value, _In_opt_ const allocator_t &alloc = allocator_t())
			: vector_t(count, value, alloc)
		{}

		/* Initializes a new instance of a Plutonium vector with a specified amount of default instances of _Ty. */
		explicit vector(_In_ size_t count, _In_opt_ const allocator_t &alloc = allocator_t())
			: vector_t(count, alloc)
		{}

		/* Initializes a new instance of a Plutonium vector with contents of the range [first, last]. */
		template <class _ItTy>
		vector(_In_ _ItTy first, _In_ _ItTy last, _In_opt_ const allocator_t &alloc = allocator_t())
			: vector_t(first, last, alloc)
		{}

		/* Copy constructor. */
		vector(_In_ const vector<_Ty> &other)
			: vector_t(other)
		{}

		/* Allocator extended copy constructor. */
		vector(_In_ const vector<_Ty> &other, _In_ const allocator_t &alloc)
			: vector_t(other, alloc)
		{}

		/* Move constructor. */
		vector(_In_ vector<_Ty> &&other) noexcept
			: vector_t(std::move(other))
		{}

		/* Allocator extended move constructor. */
		vector(_In_ vector<_Ty> &&other, _In_ const allocator_t &alloc)
			: vector_t(std::move(other), alloc)
		{}

		/* Initializes a new instance of a Plutonium vector with an initializer list. */
		vector(_In_ std::initializer_list<_Ty> init, _In_opt_ const allocator_t &alloc = allocator_t())
			: vector_t(init, alloc)
		{}

		/* Replaces the contents with a copy of the contents of other. */
		_Check_return_ inline vector<_Ty>& operator =(_In_ const vector<_Ty> &other)
		{
			if (&other != this) vector_t::operator=(other);
			return *this;
		}

		/* Moves the contents of other to this vector. */
		_Check_return_ inline vector<_Ty>& operator =(_In_ vector<_Ty> &&other)
		{
			if (&other != this) vector_t::operator=(std::move(other));
			return *this;
		}

		/* Replaces the contents with those specified by the initializer list. */
		_Check_return_ inline vector<_Ty>& operator =(_In_ std::initializer_list<_Ty> init)
		{
			vector_t::operator=(init);
			return *this;
		}

		/* Gets the iterator at the location of the specified element. */
		_Check_return_ inline const_iterator iteratorOf(_In_ const _Ty &element) const
		{
			return std::find(vector_t::begin(), vector_t::end(), element);
		}

		/* Gets the index of the specified element. */
		_Check_return_ inline size_t indexOf(_In_ const _Ty &element) const
		{
			const_iterator it = iteratorOf(element);
			if (it == vector_t::end()) ArgOutOfRange();
			return std::distance(vector_t::begin(), it);
		}

		/* Checks whether a specified element is within the vector. */
		_Check_return_ inline bool contains(_In_ const _Ty &element) const
		{
			return iteratorOf(element) != vector_t::end();
		}

		/* Removes an element the specified interator index. */
		inline void removeAt(_In_ const_iterator it)
		{
			vector_t::erase(it);
		}

		/* Removes an element at the specified index. */
		inline void removeAt(_In_ size_t idx)
		{
			vector_t::erase(vector_t::begin() + idx);
		}

		/* Removes the specified element from the vector. */
		inline void remove(_In_ const _Ty &element)
		{
			removeAt(iteratorOf(element));
		}

		/* Attempts to remove the specified element. */
		_Check_return_ inline bool tryRemove(_In_ const _Ty &element)
		{
			const_iterator it = iteratorOf(element);
			if (it == vector_t::end()) return false;

			removeAt(it);
			return true;
		}

		/* Removes all element that satisfy the predicate. */
		template <typename _ParamTy, class _PredicateTy>
		_Check_return_ inline size_t removeAll(const _ParamTy &userParam, _In_ _PredicateTy predicate)
		{
			const size_t len = vector_t::size();

			for (size_t i = 0; i < vector_t::size();)
			{
				if (predicate((*this)[i], userParam)) removeAt(i);
				else ++i;
			}

			return len - vector_t::size();
		}

	private:
		inline void ArgOutOfRange(void) const
		{
			std::_Xout_of_range("Index was out of range!");
		}
	};
}