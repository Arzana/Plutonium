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

		/* Gets whether this asset has a name. */
		_Check_return_ inline bool HasName(void) const
		{
			return !identifier.empty();
		}

		/* Gets the human readable indentifier of the asset. */
		_Check_return_ inline const wstring& GetName(void) const
		{
			return identifier;
		}

		/* Increases the reference count by one. */
		inline void Reference(void)
		{
			++refCnt;
		}

		/* Marks the current asset as ready for use. */
		inline void MarkAsLoaded(_In_ wstring &&name)
		{
			MarkAsLoaded(false, std::move(name));
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
		inline void MarkAsLoaded(_In_ bool viaLoader, _In_ wstring &&name)
		{
			loaded.store(true);
			loadedViaLoader = viaLoader;
			identifier = std::move(name);
		}

	private:
		friend class AssetCache;
		friend class AssetLoader;
		friend class AssetFetcher;

		int32 refCnt;
		size_t hash, instance;
		bool allowDuplication, loadedViaLoader;
		std::atomic_bool loaded;
		wstring identifier;

		template <typename asset_t>
		asset_t& Duplicate(AssetCache &cache)
		{
			if (allowDuplication) return static_cast<asset_t&>(Duplicate(cache));

			Log::Error("Asset '%zu' (%s) denied being copied!", hash, typeid(asset_t).name());
			return static_cast<asset_t&>(*this);
		}
	};
}