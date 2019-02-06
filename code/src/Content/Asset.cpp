#include "Content/Asset.h"
#include "Core/Diagnostics/Logging.h"

Pu::Asset::~Asset(void)
{
	if (refCnt > 0) Log::Warning("Releasing referenced asset '%zu'!", hash);
}

Pu::Asset::Asset(DuplicationType type)
	: Asset(type, 0)
{}

Pu::Asset::Asset(DuplicationType type, size_t hash)
	: Asset(type, hash, 0)
{}

Pu::Asset::Asset(DuplicationType type, size_t hash, size_t instance)
	: refCnt(1), hash(hash), instance(instance), type(type), loaded(false)
{}

Pu::Asset::Asset(Asset && value)
	: refCnt(value.refCnt), hash(value.hash), instance(value.instance), type(value.type), loaded(value.IsLoaded())
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
		if (refCnt > 0) Log::Warning("Overriding referenced asset '%zu'!", hash);

		refCnt = other.refCnt;
		hash = other.hash;
		instance = other.instance;
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
	if (this->hash) Log::Fatal("Cannot reset asset hash!");
	this->hash = hash;
}
#pragma warning(pop)

Pu::Asset & Pu::Asset::Duplicate(AssetCache & cache)
{
	if (type == DuplicationType::Reference)
	{
		++refCnt;
		return *this;
	}

	return MemberwiseCopy(cache);
}