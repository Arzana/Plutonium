#include "Content/AssetCache.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Threading/PuThread.h"
#include <imgui/include/imgui.h>

Pu::AssetCache::AssetCache(AssetCache && value)
{
	/* Lock both mutexes for safety. */
	lock.lock();
	value.lock.lock();

	assets = std::move(value.assets);
	reserved = std::move(value.reserved);

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
		reserved = std::move(other.reserved);

		/* Unlock both mutexes again. */
		other.lock.unlock();
		lock.lock();
	}

	return *this;
}

size_t Pu::AssetCache::RngHash(void)
{
	/* Make sure that we're the only ones accessing the cache. */
	lock.lock();
	size_t result;
	std::hash<string> hasher;

	/*
	Just create random 64-length string hashes until we find one this isn't used yet.
	By either the hash or the instance hash.
	*/
	do
	{
		result = hasher(random(64));
	} while (Contains(result, true, true, true));

	/* Reserve the hash and return it to the user. */
	reserved.emplace_back(result);
	lock.unlock();
	return result;
}

bool Pu::AssetCache::Reserve(size_t hash)
{
	lock.lock();

	/*
	Spin if the specified hash has already been reserved,
	but isn't in the full asset cache yet.
	*/
	while (Contains(hash, false, false, true))
	{
		lock.unlock();
		PuThread::Sleep(1);
		lock.lock();
	}

	/*
	Check whether the asset is in the cache,
	if so we can't reserve it so return false.
	*/
	if (Contains(hash, false, true, false))
	{
		lock.unlock();
		return false;
	}

	/* Otherwise we reserve the hash and return true. */
	reserved.emplace_back(hash);
	lock.unlock();
	return true;
}

bool Pu::AssetCache::Contains(size_t hash, bool instance, bool asset, bool reserve) const
{
	if (asset)
	{
		for (const Asset *cur : assets)
		{
			if (cur->hash == hash)
			{
				if (!instance)  return true;
				if (cur->instance == hash) return true;
			}
		}
	}

	if (reserve)
	{
		for (size_t cur : reserved)
		{
			if (cur == hash) return true;
		}
	}

	return false;
}

/* Not all codepaths return a value, Log::Fatal will always throw. */
#pragma warning(push)
#pragma warning(disable:4715)
Pu::Asset & Pu::AssetCache::Get(size_t hash)
{
	lock.lock();

	for (Asset *cur : assets)
	{
		if (cur->hash == hash)
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
	Log::Warning("Attempting to release unknown asset '%zu'!", asset.hash);
}

void Pu::AssetCache::Store(Asset * asset)
{
	lock.lock();

	/* Only check if the asset is already added if the hash isn't zero (indicates hash hasn't been set yet). */
	if (asset->hash != 0 && Contains(asset->hash, true, true, false))
	{
		lock.unlock();
		Log::Error("Attempting to add already added asset %zu!", asset->hash);
		return;
	}

	/* Make sure to remove the asset from the reserve list, and throw a warning if it was just added without reserving. */
	if (Contains(asset->hash, false, false, true)) reserved.remove(asset->hash);
	else if (Contains(asset->instance, false, false, true)) reserved.remove(asset->instance);
	else Log::Warning("Asset %zu was added before first reserving it, this is unsafe!", asset->hash);

	assets.emplace_back(asset);
	lock.unlock();
}

void Pu::AssetCache::Update(Asset * asset, size_t newHash)
{
	lock.lock();

	/* Remove the new hash from the reserve list. */
	if (Contains(newHash, false, false, true)) reserved.remove(newHash);
	else Log::Warning("Asset %zu has updated its hash before first reserving it, this is unsafe!", newHash);

	/* Update the hash safely. */
	asset->SetHash(newHash);
	lock.unlock();
}

void Pu::AssetCache::Visualize(void) const
{
	if constexpr (ImGuiAvailable)
	{
		if (ImGui::Begin("Assets"))
		{
			const ImVec4 clr = Color::Astronaut().ToVector4();

			lock.lock();
			ImGui::Text("Total Assets %zu", assets.size());

			ImGui::Columns(3);
			ImGui::SetColumnWidth(-1, 100.0f);
			ImGui::TextColored(clr, "Hash");
			for (const Asset *asset : assets)
			{
				ImGui::Text("0x%X", asset->hash);
			}

			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 300.0f);
			ImGui::TextColored(clr, "Name");
			for (const Asset *asset : assets)
			{
				ImGui::Text("%ls", asset->identifier.c_str());
			}

			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 100.0f);
			ImGui::TextColored(clr, "References");
			for (const Asset *asset : assets)
			{
				ImGui::Text("%d", asset->refCnt);
			}

			lock.unlock();
			ImGui::End();
		}
	}
}