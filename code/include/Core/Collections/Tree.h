#pragma once
#include <vector>
#include "Core\SafeMemory.h"

namespace Plutonium
{
	/* Defines a generic size tree. */
	template <typename _Ty>
	struct Tree
	{
	public:
		/* Initializes a new instance of an empty tree. */
		Tree(void)
			: data(nullptr), size(0), count(0), branch(0)
		{}

		/* Releases the resources allocated by the tree. */
		~Tree(void) noexcept
		{
			/* Only clear if data has been set at some point. */
			if (data)
			{
				free_s(data);
				size = 0;
				count = 0;
			}
		}

		/* Copies the structure of the current tree to a new tree of the specified type. */
		template <typename _DestTy>
		void CloneStructure(_Inout_ Tree<_DestTy> *result) const
		{
			/* Initialize result. */
			result->EnsureCapacity(count);
			result->count = count;

			/* Copy internal structure. */
			for (size_t i = 0; i < count; i++)
			{
				result->data[i].Init<_Ty>(data + i);
			}
		}

		/* Gets whether the current branch has child branches. */
		_Check_return_ bool HasBranch(void) const
		{
			ASSERT_IF(count <= 0, "Tree contains no branches!");
			return data[branch].Childs.size() > 0;
		}

		/* Gets whether the current branch is the bottom most. */
		_Check_return_ bool IsStump(void) const
		{
			ASSERT_IF(count <= 0, "Tree contains no branches!");
			return branch == 0;
		}

		/* Moves down one parent in the tree. */
		void ClimbDown(void)
		{
			ASSERT_IF(IsStump(), "Cannot climb down tree any further!");
			branch = data[branch].ParentIndex;
		}

		/* Moves up one child in the tree, selecting the first child. */
		void ClimbUp(void)
		{
			ASSERT_IF(!HasBranch(), "Cannot climb up tree any further!");
			branch = data[branch].Childs.at(0);
		}

		/* Moves over to the next branch; returns false if unable. */
		_Check_return_ bool NextBranch(void)
		{
			/* Stump never has neighbor branches. */
			if (IsStump()) return false;

			/* Save current child for later. */
			size_t old = branch;
			ClimbDown();

			/* Loop though childs to find old child. */
			for (size_t i = 0; i < data[branch].Childs.size(); i++)
			{
				/* if child is not last child return true and set branch. */
				size_t cur = data[branch].Childs.at(i);
				if (cur == old)
				{
					if (i + 1 < data[branch].Childs.size())
					{
						branch = data[branch].Childs.at(i + 1);
						return true;
					}
					else return false;
				}
			}

			LOG_THROW("Could not find old branch in tree, this should never occur!");
			return false;
		}

		/* Adds a branch to the current branch or creates a stump when the tree is empty. */
		void Add(_In_ _Ty value)
		{
			EnsureCapacity(count + 1);
			data[count].Init(value, branch);
			if (count > 0) data[branch].Childs.push_back(count);
			count++;
		}

		/* Gets the value of the current branch. */
		_Check_return_ _Ty& Value(void) const
		{
			ASSERT_IF(count <= 0, "Tree is empty!");
			return data[branch].Value;
		}

		/* Gets the current capacity of the tree. */
		_Check_return_ size_t GetCapacity(void) const
		{
			return size;
		}

		/* Gets the current branch count of the tree. */
		_Check_return_ size_t GetCount(void) const
		{
			return count;
		}

	private:
		template <typename>
		friend struct Tree;

		struct Branch
		{
		public:
			typename std::remove_const<_Ty>::type Value;
			size_t ParentIndex;
			std::vector<size_t> Childs;

			void Init(_Ty value, size_t parent)
			{
				Value = value;
				ParentIndex = parent;
				ValidateVector();
			}

			template <typename _SrcTy>
			void Init(const typename Tree<typename _SrcTy>::Branch *old)
			{
				ParentIndex = old->ParentIndex;
				ValidateVector();
				Childs = std::vector<size_t>(old->Childs);
			}

		private:
			void ValidateVector(void)
			{
				/* Because we're allocaing the vector from random memory we have to make sure the iterator is default. */
#if defined(DEBUG)
				Childs._Get_data()._Myproxy = nullptr;
#endif
				Childs._Get_data()._Myfirst = nullptr;
				Childs._Get_data()._Mylast = nullptr;
				Childs._Get_data()._Myend = nullptr;
				Childs.clear();
			}
		};

		Branch *data;
		size_t size;
		size_t count;
		size_t branch;

		void EnsureCapacity(size_t required)
		{
			if (required <= size) return;

			if (size == 0) data = malloc_s(Branch, required);
			else realloc_s(Branch, data, required);

			size = required;
		}
	};
}