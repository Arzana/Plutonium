#pragma once
#include "Core/Diagnostics/Logging.h"

namespace Pu
{
	/* Defines a reference count object for generic (this objects owns the assets). */
	struct SharedAsset
	{
	public:
		/* Initializes a new instance of a shared asset. */
		template <typename asset_t>
		SharedAsset(_In_ asset_t *asset)
			: refCnt(1), asset(asset), typeHash(typeid(asset_t).hash_code())
#ifdef _DEBUG
			, typeName(typeid(asset_t).name())
#endif
		{}

		SharedAsset(_In_ const SharedAsset&) = delete;

		/* Move constructor. */
		SharedAsset(_In_ SharedAsset &&value)
			: refCnt(value.refCnt), asset(value.asset), typeHash(value.typeHash)
#ifdef _DEBUG
			, typeName(std::move(value.typeName))
#endif
		{
			value.asset = nullptr;
			value.refCnt = 0;
		}

		/* Deletes the asset. */
		~SharedAsset(void)
		{
			Delete<void>();
		}

		_Check_return_ SharedAsset& operator =(_In_ const SharedAsset&) = delete;
		_Check_return_ SharedAsset& operator =(_In_ SharedAsset&&) = delete;

		/* Gets the underlyng asset as the correct type and increases its reference count. */
		template <typename asset_t>
		_Check_return_ inline asset_t& Reference(void)
		{
			IncreaseReference();
			return Get<asset_t>();
		}

		/* Dereferences the underlying asset and returns whether it can be deleted. */
		_Check_return_ inline bool DeReference(void)
		{
			return --refCnt <= 0;
		}

		/* Deletes the asset (cannot delete void pointer with destructor call). */
		template <typename asset_t>
		void Delete(void)
		{
			if (refCnt > 0) Log::Warning("Deleting still referenced asset!");
			if (asset)
			{
				delete reinterpret_cast<asset_t*>(asset);
				asset = nullptr;
			}
		}

	protected:
		/* Gets a mutable reference to the asset. */
		template <typename asset_t>
		_Check_return_ inline asset_t& Get(void)
		{
			if (typeid(asset_t).hash_code() != typeHash) Log::Fatal("Unable to cast asset to %s!", typeid(asset_t).name());
			return *reinterpret_cast<asset_t*>(asset);
		}

		/* Increases the reference count by one. */
		inline void IncreaseReference(void)
		{
			++refCnt;
		}

	private:
		const size_t typeHash;
		int32 refCnt;
		void *asset;

#ifdef _DEBUG
		/* This only exists for the visualizer. */
		const string typeName;
#endif
	};
}