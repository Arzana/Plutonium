#include "Content/AssetCache.h"
#include "Core/Diagnostics/Logging.h"

Pu::AssetCache::AssetCache(AssetCache && value)
{
	/* Lock both mutexes for safety. */
	lock.lock();
	value.lock.lock();

	assets = std::move(value.assets);

	/* Unlock both mutexes again. */
	value.lock.unlock();
	lock.unlock();
}

Pu::AssetCache & Pu::AssetCache::operator=(AssetCache && other)
{
	if (this != &other)
	{
		/* Lock both mutexes for safety. */
		lock.lock();
		other.lock.lock();

		assets = std::move(other.assets);

		/* Unlock both mutexes again. */
		other.lock.unlock();
		lock.lock();
	}

	return *this;
}

size_t Pu::AssetCache::RngHash(void) const
{
	size_t result;
	std::hash<string> hasher;

	do
	{
		result = hasher(random(64));
	} while (Contains(result));

	return result;
}

bool Pu::AssetCache::Contains(size_t hash) const
{
	lock.lock();
	const bool result = assets.contains([hash](const Asset *cur) { return *cur == hash; });
	lock.unlock();

	return result;
}

/* Not all codepaths return a value, Log::Fatal will always throw. */
#pragma warning(push)
#pragma warning(disable:4715)
Pu::Asset & Pu::AssetCache::Get(size_t hash)
{
	lock.lock();

	for (Asset *cur : assets)
	{
		if (*cur == hash)
		{
			lock.unlock();
			return *cur;
		}
	}

	Log::Fatal("Attempting to retrieve unknown asset: '%zu'!", hash);
}
#pragma warning(pop)

void Pu::AssetCache::Release(Asset & asset)
{
	lock.lock();

	for (Asset *cur : assets)
	{
		if (*cur == asset)
		{
			if (--cur->refCnt < 1)
			{
				if (!cur->loadedViaLoader) Log::Warning("Releasing known asset which was not loaded via loader (is it still loading?)!");
				assets.remove(cur);
				delete cur;
			}
			lock.unlock();
			return;
		}
	}

	lock.unlock();
	if (!asset.loadedViaLoader) Log::Warning("Attempting to release unknown asset '%zu'!", asset.hash);
}

void Pu::AssetCache::Store(Asset * asset)
{
	/* Only check if the asset is already added if the hash isn't zero (indicates hash hasn't been set yet). */
	if (asset->hash != 0 && Contains(asset->hash))
	{
		Log::Error("Attempting to add already added asset '%zu'!", asset->hash);
		return;
	}

	lock.lock();
	assets.push_back(asset);
	lock.unlock();
}