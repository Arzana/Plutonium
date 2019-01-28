#pragma once
#include <mutex>
#include "SavedAsset.h"

namespace Pu
{
	/* Defines a container for saved main assets. */
	class AssetLibrary
	{
	public:
		/* Defines the result type of an GetOrNew operation. */
		template <typename asset_t>
		using result_t = std::tuple<bool, asset_t&>;

		/* Initializes a new instance of an asset library. */
		AssetLibrary(void) = default;
		AssetLibrary(_In_ const AssetLibrary&) = delete;
		/* Move constructor. */
		AssetLibrary(_In_ AssetLibrary &&value)
			: lib(std::move(value.lib))
		{}

		_Check_return_ AssetLibrary& operator =(_In_ const AssetLibrary&) = delete;
		_Check_return_ AssetLibrary& operator =(_In_ AssetLibrary&&) = delete;

		/* Attempts to get the specified asset from the library, otherwise; creates a new one. */
		template <typename asset_t, DuplicationType type = DuplicationType::Reference, typename ... ctor_args_t>
		_Check_return_ result_t<asset_t> GetOrNew(_In_ size_t hash, _In_opt_ ctor_args_t ... args)
		{
			lock.lock();

			/* Check if the library contains the asset. */
			std::map<size_t, SavedAsset>::iterator it = lib.find(hash);
			if (it != lib.end())
			{
				lock.unlock();

				/* If the asset is a reference duplicate asset; reference it. */
				if (it->second.GetType() == DuplicationType::Reference)
				{
					return result_t<asset_t>(true, it->second.Reference<asset_t>());
				}
				else if (it->second.GetType() == DuplicationType::MemberwiseCopy)
				{
					/* Create a new copy of the asset and save it as a memberwise copy asset. */
					asset_t *copy = it->second.MemberwiseCopy<asset_t>();
					Store(hash, copy, DuplicationType::MemberwiseCopy);

					return result_t<asset_t>(true, *copy);
				}
				else Log::Fatal("AssetLibrary doesn't support duplication type!");
			}

			/* Asset wasn't found so create a new one. */
			asset_t *asset = new asset_t(args...);
			Store(hash, asset, type);

			lock.unlock();
			return result_t<asset_t>(false, *asset);
		}

		/* Frees a specific asset from the library. */
		template <typename asset_t>
		inline void Free(_In_ size_t hash)
		{
			lock.lock();
			std::map<size_t, SavedAsset>::iterator it = lib.find(hash);

			if (it != lib.end())
			{
				if (it->second.DeReference())
				{
					it->second.Delete<asset_t>();
					lib.erase(it);
				}
			}
			else Log::Warning("Attempting to free unknown asset!");

			lock.unlock();
		}

	private:
		std::map<size_t, SavedAsset> lib;
		std::mutex lock;

		template <typename asset_t>
		inline void Store(size_t hash, asset_t *asset, DuplicationType type)
		{
			lib.emplace(hash, SavedAsset(asset, type));
		}
	};
}