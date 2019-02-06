#pragma once
#include <atomic>
#include "DuplicationType.h"
#include "Core/Math/Constants.h"

namespace Pu
{
	class AssetCache;

	/* Defines a base class for asset types. */
	class Asset
	{
	public:
		Asset(_In_ const Asset&) = delete;
		/* Releases the resources allocated by the asset. */
		virtual ~Asset(void);

		_Check_return_ Asset& operator =(_In_ const Asset&) = delete;

		/* Checks whether two assets are equal. */
		_Check_return_ inline bool operator ==(_In_ const Asset &other) const 
		{
			return other.hash == hash && other.instance == instance;
		}

		/* Checks whether two assets differ. */
		_Check_return_ inline bool operator !=(_In_ const Asset &other) const 
		{
			return other.hash != hash || other.instance != instance;
		}

		/* Checks whether the hash is equal to the hash of the asset. */
		_Check_return_ inline bool operator ==(_In_ size_t other) const
		{
			return other == hash;
		}

		/* Checks whether the hash differs from the hash of the asset. */
		_Check_return_ inline bool operator !=(_In_ size_t other) const
		{
			return other != hash;
		}

		/* Gets whether the asset is marked as usable. */
		_Check_return_ inline bool IsLoaded(void) const
		{
			return loaded.load();
		}

	protected:
		/* Initiaizes a new instance of an asset. */
		Asset(_In_ DuplicationType type);
		/* Initializes a new instance of an asset with a specific hash. */
		Asset(_In_ DuplicationType type, _In_ size_t hash);
		/* Initializes a new instance of an asset with a specified hash and instance hash. */
		Asset(_In_ DuplicationType type, _In_ size_t hash, _In_ size_t instance);
		/* Move constructor. */
		Asset(_In_ Asset &&value);

		/* Move assignment. */
		_Check_return_ Asset& operator =(_In_ Asset &&other);

		/* Gets a memberwise copy of the asset, this won't be called if the duplication type is reference. */
		_Check_return_ virtual Asset& MemberwiseCopy(_In_ AssetCache &cache) = 0;
		/* Initializes the hash of the asset, cannot replace valid hash! */
		void SetHash(_In_ size_t hash);
		/* Marks the current asset as ready for use. */
		inline void MarkAsLoaded(void)
		{
			loaded.store(true);
		}

	private:
		friend class AssetCache;
		friend class AssetLoader;
		friend class AssetFetcher;

		mutable int32 refCnt;
		size_t hash, instance;
		DuplicationType type;
		std::atomic_bool loaded;

		Asset& Duplicate(AssetCache &cache);

		template <typename asset_t>
		asset_t& Duplicate(AssetCache &cache)
		{
			return static_cast<asset_t&>(Duplicate(cache));
		}
	};
}