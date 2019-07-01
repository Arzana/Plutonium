#include "Content/Asset.h"

Pu::Asset::~Asset(void)
{
	/*
	Assets that aren't allowed to be duplicated are assets whose memory is handled by another system (like the OS).
	So these will always have one reference and need to be de-referened by the user.
	The same goes for assets that were loaded by the user without the loader.
	They are responsible for safe releases.
	All other assets should be released via the asset fetcher, thusly removing its references.
	*/
	if (allowDuplication && loadedViaLoader && refCnt > 0) Log::Warning("Releasing referenced asset '%zu'!", hash);
}

Pu::Asset::Asset(bool allowDuplication)
	: Asset(allowDuplication, 0)
{}

Pu::Asset::Asset(bool allowDuplication, size_t hash)
	: Asset(allowDuplication, hash, 0)
{}

Pu::Asset::Asset(bool allowDuplication, size_t hash, size_t instance)
	: refCnt(1), hash(hash), instance(instance), allowDuplication(allowDuplication), loaded(false), loadedViaLoader(false)
{}

Pu::Asset::Asset(Asset && value)
	: refCnt(value.refCnt), hash(value.hash), instance(value.instance), 
	allowDuplication(value.allowDuplication), loaded(value.IsLoaded()), identifier(std::move(value.identifier))
{
	value.refCnt = 0;
	value.hash = 0;
	value.instance = 0;
	value.loaded.store(false);
}

Pu::Asset & Pu::Asset::operator=(Asset && other)
{
	if (this != &other)
	{
		if (allowDuplication && loadedViaLoader && refCnt > 0) Log::Warning("Overriding referenced asset '%zu'!", hash);

		refCnt = other.refCnt;
		hash = other.hash;
		instance = other.instance;
		identifier = std::move(other.identifier);
		allowDuplication = other.allowDuplication;
		loaded.store(other.IsLoaded());

		other.refCnt = 0;
		other.hash = 0;
		other.instance = 0;
		other.loaded.store(false);
	}

	return *this;
}

/* Warning checked and nothing is wrong. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Asset::SetHash(size_t hash)
{
	this->hash = hash;
}
#pragma warning(pop)