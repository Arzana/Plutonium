#pragma once
#include <map>
#include <mutex>
#include "SharedAsset.h"

namespace Pu
{
	/* Defines a storage object for shared assets. */
	class IoCache
	{
	public:
		/* Initializes a new instance of an I/O cache. */
		IoCache(void) = default;
		IoCache(_In_ const IoCache&) = delete;
		/* Move constructor. */
		IoCache(_In_ IoCache &&value)
			: cache(std::move(value.cache))
		{}

		_Check_return_ IoCache& operator =(_In_ const IoCache&) = delete;
		_Check_return_ IoCache& operator =(_In_ IoCache&&) = delete;

		/* Checks whether a specific asset is cached. */
		_Check_return_ inline bool Contains(_In_ size_t hash) const
		{
			lock.lock();
			const bool result = cache.find(hash) != cache.end();
			lock.unlock();

			return result;
		}

		/* Gets a new reference to the specific asset. */
		template <typename asset_t>
		_Check_return_ inline asset_t& Get(_In_ size_t hash)
		{
			lock.lock();
			asset_t &result = cache.find(hash)->second.Reference<asset_t>();
			lock.unlock();

			return result;
		}

		/* Adds a new asset to the cache. */
		template <typename asset_t>
		inline void Add(_In_ asset_t *asset, _In_ size_t hash)
		{
			lock.lock();
			cache.emplace(hash, asset);
			lock.unlock();
		}

		/* Dereferences the asset associated with the hash. */
		template <typename asset_t>
		_Check_return_ inline void Free(_In_ size_t hash)
		{
			lock.lock();
			std::map<size_t, SharedAsset>::iterator it = cache.find(hash);

			if (it != cache.end())
			{
				if (it->second.DeReference())
				{
					it->second.Delete<asset_t>();
					cache.erase(it);
				}
			}
			else Log::Warning("Attempting to free unknown asset!");

			lock.unlock();
		}

	private:
		std::map<size_t, SharedAsset> cache;
		mutable std::mutex lock;
	};
}