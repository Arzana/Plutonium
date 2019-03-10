#pragma once
#include <atomic>
#include "Core/Diagnostics/Logging.h"

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

		/* Increases the reference count by one. */
		inline void Reference(void)
		{
			++refCnt;
		}

	protected:
		/* Initiaizes a new instance of an asset. */
		Asset(_In_ bool allowDuplication);
		/* Initializes a new instance of an asset with a specific hash. */
		Asset(_In_ bool allowDuplication, _In_ size_t hash);
		/* Initializes a new instance of an asset with a specified hash and instance hash. */
		Asset(_In_ bool allowDuplication, _In_ size_t hash, _In_ size_t instance);
		/* Move constructor. */
		Asset(_In_ Asset &&value);

		/* Move assignment. */
		_Check_return_ Asset& operator =(_In_ Asset &&other);

		/* Duplicates the asset, either returning a reference of itself or a memberwise copy. */
		_Check_return_ virtual Asset& Duplicate(_In_ AssetCache &cache) = 0;
		/* Initializes the hash of the asset, cannot replace valid hash! */
		void SetHash(_In_ size_t hash);

		/* Marks the current asset as ready for use. */
		inline void MarkAsLoaded(_In_ bool viaLoader)
		{
			loaded.store(true);
			loadedViaLoader = viaLoader;
		}

	private:
		friend class AssetCache;
		friend class AssetLoader;
		friend class AssetFetcher;

		int32 refCnt;
		size_t hash, instance;
		bool allowDuplication, loadedViaLoader;
		std::atomic_bool loaded;

		template <typename asset_t>
		asset_t& Duplicate(AssetCache &cache)
		{
			if (allowDuplication) return static_cast<asset_t&>(Duplicate(cache));

			Log::Error("Asset '%zu' denied being copied!", hash);
			return static_cast<asset_t&>(*this);
		}
	};
}