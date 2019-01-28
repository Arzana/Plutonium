#pragma once
#include "SharedAsset.h"

namespace Pu
{
	/* Defines the types of duplication for saved assets. */
	enum class DuplicationType
	{
		/* Asset shouldn't be copied but instead referenced. */
		Reference,
		/* Asset should have specific members copied. */
		MemberwiseCopy
	};

	/* Defines a reference count saved asset that can be duplicated. */
	struct SavedAsset
		: public SharedAsset
	{
	public:
		/* Initializes a new instance of a saved asset. */
		template <typename asset_t>
		SavedAsset(_In_ asset_t *asset, _In_ DuplicationType type)
			: SharedAsset(asset), type(type)
		{}

		SavedAsset(_In_ const SavedAsset&) = delete;

		/* Move constructor. */
		SavedAsset(_In_ SavedAsset &&value)
			: SharedAsset(std::move(value)), type(value.type)
		{}

		_Check_return_ SharedAsset& operator =(_In_ const SharedAsset&) = delete;
		_Check_return_ SharedAsset& operator =(_In_ SharedAsset&&) = delete;

		/* Gets the duplication type of this saved asset. */
		_Check_return_ inline DuplicationType GetType(void) const 
		{
			return type;
		}

		/* Creates a memberwise copy of the asset (requires delete!). */
		template <typename asset_t>
		_Check_return_ asset_t* MemberwiseCopy(void)
		{
			if (type != DuplicationType::MemberwiseCopy) Log::Fatal("Attempting to memberwise copy reference asset!");
			return Get<asset_t>().Copy();
		}

	private:
		DuplicationType type;
	};
}