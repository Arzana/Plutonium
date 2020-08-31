#pragma once
#include <mutex>
#include "Asset.h"
#include "Core/Collections/vector.h"

namespace Pu
{
	/* Defines a collection of assets. */
	class AssetCache
	{
	public:
		/* Initializes a new instance of an asset cache. */
		AssetCache(void) = default;
		AssetCache(_In_ const AssetCache&) = delete;
		/* Move constructor. */
		AssetCache(_In_ AssetCache &&value);

		_Check_return_ AssetCache& operator =(_In_ const AssetCache&) = delete;
		/* Move assignment. */
		_Check_return_ AssetCache& operator =(_In_ AssetCache &&other);

		/* 
		Creates a random hash that isn't yet used by the asset cache. 
		Set the base hash if you wish to check for instance hashes.
		*/
		_Check_return_ size_t RngHash(_In_opt_ size_t baseHash = 0);

		/* Attempts to reserve the specific asset hash, returns false if it already exists. */
		_Check_return_ bool Reserve(_In_ size_t hash);
		/* Gets the asset with the specified hash. */
		_Check_return_ Asset& Get(_In_ size_t hash);

		/* Releases an instance of an asset, will only delete the asset if it's not referenced anymore. */
		void Release(_In_ Asset &asset);
		/* Stores a new asset in the cache. */
		void Store(_In_ Asset *asset);
		/* Updates the hash of a reserved asset in tha cache. */
		void Update(_In_ Asset *asset, _In_ size_t newHash);
		/* Visualizes the assets currently active in this cache store. */
		void Visualize(void) const;

	private:
		mutable std::mutex lock;
		vector<Asset*> assets;
		vector<size_t> reserved;

		bool Contains(size_t hash, bool asset, bool reserve) const;
		bool Contains(size_t base, size_t hash) const;
	};
}