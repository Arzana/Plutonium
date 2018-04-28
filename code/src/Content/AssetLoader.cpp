#pragma warning(disable:4996)

#include "Content\AssetLoader.h"
#include "Core\Threading\TickThread.h"
#include "Core\Threading\ThreadUtils.h"
#include "Core\StringFunctions.h"
#include <atomic>

using namespace Plutonium;

/* Defines temporary holding space for immadiate loading of assets. */
template <typename _Ty>
struct LoadResult
{
	std::atomic_bool loaded;
	_Ty *value;

	LoadResult(void)
		: loaded(false), value(nullptr)
	{}

	void OnLoadComplete(const AssetLoader*, _Ty *tex)
	{
		value = tex;
		loaded.store(true);
	}
};

Plutonium::AssetLoader::AssetLoader(WindowHandler wnd, const char * root)
	: wnd(wnd), root(heapstr(root)), rootLen(strlen(root)), state(heapstr("Initializing..."))
{
	/* Create new IO thread per asset loader. */
	ioThread = new TickThread("PuIO");
	ioThread->Initialize.Add(this, &AssetLoader::IoThreadInit);

	/* Add specified loader methods to the tick queue. */
	ioThread->Tick.Add(this, &AssetLoader::TickIoTextures);
	ioThread->Tick.Add(this, &AssetLoader::TickIoModels);

	/* Start IO thread. */
	ioThread->Start();
}

Plutonium::AssetLoader::~AssetLoader(void) noexcept
{
	/* Make sure we wait until the thread is done loading to not corrupt memory. */
	ioThread->StopWait();

	/* Release strings. */
	free_s(root);
	free_s(state);

	/* Unload all stored textures. */
	while (loadedTextures.size())
	{
		AssetInfo<Texture> *cur = loadedTextures.back();
		LOG_WAR_IF(cur->RefCnt > 0, "Active refrence to texture '%s' is removed!", cur->Asset->GetName());
		delete_s(cur);
		loadedTextures.pop_back();
	}

	/* Unload all stored models. */
	while (loadedModels.size())
	{
		AssetInfo<StaticModel> *cur = loadedModels.back();
		LOG_WAR_IF(cur->RefCnt > 0, "Active refrence to model is removed!");
		delete_s(cur);
		loadedModels.pop_back();
	}
}

const char * Plutonium::AssetLoader::GetState(void) const
{
	/* Safely get a copy of the current state. */
	lockState.lock();
	const char *result = heapstr(state);
	lockState.unlock();

	return result;
}

void Plutonium::AssetLoader::SetRoot(const char * root)
{
	/* Safely sets the root directory (and size) to a copy of the specified path. */
	lockRoot.lock();
	free_s(this->root);
	this->root = heapstr(root);
	rootLen = strlen(root);
	lockRoot.unlock();
}

bool Plutonium::AssetLoader::Unload(const char * path)
{
	if (!path) return false;

	/* Check if asset is texture. */
	lockTex.lock();
	int32 idx = GetTextureIdx(path);
	if (idx != -1)
	{
		AssetInfo<Texture> *cur = loadedTextures.at(idx);
		LOG_WAR_IF(cur->RefCnt <= 0, "Attempting to unload unrefrenced texture!");

		/* Remove refrence to texture and checks if it needs to be deleted. */
		if ((--cur->RefCnt) <= 0 && !cur->Keep)
		{
			/* Delete texture and remove it from the list. */
			loadedTextures.erase(loadedTextures.begin() + idx);
			lockTex.unlock();
			delete_s(cur);
			return true;
		}

		lockTex.unlock();
		return true;
	}
	lockTex.unlock();

	/* Check if asset is model. */
	lockMod.lock();
	idx = GetModelIdx(path);
	if (idx != -1)
	{
		AssetInfo<StaticModel> *cur = loadedModels.at(idx);
		LOG_WAR_IF(cur->RefCnt <= 0, "Attempting to unload unrefrenced model!");

		/* Remove refrence to model and check if it needs to be deleted. */
		if ((--cur->RefCnt) <= 0 && !cur->Keep)
		{
			/* Delete model and remove it from the list. */
			loadedModels.erase(loadedModels.begin() + idx);
			lockMod.unlock();
			delete_s(cur);
			return true;
		}

		lockMod.unlock();
		return true;
	}
	lockMod.unlock();

	return false;
}

void Plutonium::AssetLoader::LoadTexture(const char * path, EventSubscriber<AssetLoader, Texture*>* callback, bool keep, TextureCreationOptions * config)
{
	lockTex.lock();

	int32 idx = GetTextureIdx(path);
	if (idx != -1)
	{
		/* If texture is already loaded return it and increase it's refrence count. */
		AssetInfo<Texture> *cur = loadedTextures.at(idx);
		++cur->RefCnt;
		callback->HandlePost(this, cur->Asset);
		lockTex.unlock();
	}
	else
	{
		/* We have to load the texture so create an infomation struct. */
		TextureLoadInfo *info = new TextureLoadInfo(new FileReader(path, true), keep, callback, config);

		/* Make sure we actually load the texture on the IO thread. */
		if (OnIoThread())
		{
			lockTex.unlock();
			LoadTextureInternal(info, true);
		}
		else
		{
			queuedTextures.push(info);
			lockTex.unlock();
		}
	}
}

void Plutonium::AssetLoader::LoadModel(const char * path, EventSubscriber<AssetLoader, StaticModel*>* callback, bool keep)
{
	lockMod.lock();

	int32 idx = GetModelIdx(path);
	if (idx != -1)
	{
		/* if model is already loaded return it and increase it's refrence count. */
		AssetInfo<StaticModel> *cur = loadedModels.at(idx);
		++cur->RefCnt;
		callback->HandlePost(this, cur->Asset);
		lockMod.unlock();
	}
	else
	{
		/* We have to load the model so create an information struct. */
		AssetLoadInfo<StaticModel> *info = new AssetLoadInfo<StaticModel>(new FileReader(path, true), keep, callback);

		/* Make sure we actually load the model on the IO thread. */
		if (OnIoThread())
		{
			lockMod.unlock();
			LoadModelInternal(info, true);
		}
		else
		{
			queuedModels.push(info);
			lockMod.unlock();
		}
	}
}

Texture * Plutonium::AssetLoader::LoadTexture(const char * path, bool keep, TextureCreationOptions * config)
{
	/* Create temporary storage. */
	LoadResult<Texture> result;

	/* Load the texture with the specified callback. */
	EventSubscriber<AssetLoader, Texture*> callback(&result, &LoadResult<Texture>::OnLoadComplete);
	LoadTexture(path, &callback, keep, config);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

StaticModel * Plutonium::AssetLoader::LoadModel(const char * path, bool keep)
{
	/* Create temporary storage. */
	LoadResult<StaticModel> result;

	/* Load the texture with the specified callback. */
	EventSubscriber<AssetLoader, StaticModel*> callback(&result, &LoadResult<StaticModel>::OnLoadComplete);
	LoadModel(path, &callback, keep);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

const char * Plutonium::AssetLoader::CreateFullPath(const char * fpath)
{
	lockRoot.lock();

	/* If string already contains the root skip it. */
	if (strstr(fpath, root))
	{
		lockRoot.unlock();
		char *buffer = malloca_s(char, strlen(fpath) + 1);
		strcpy(buffer, fpath);
		return buffer;
	}

	/* Merge the two strings into a stack path. */
	char *buffer = malloca_s(char, rootLen + strlen(fpath) + 1);
	mrgstr(root, fpath, buffer);
	lockRoot.unlock();

	return buffer;
}

void Plutonium::AssetLoader::SetStateWaiting(void)
{
	SetState("Waiting...");
}

void Plutonium::AssetLoader::SetStateLoading(const char * asset)
{
	char *buffer = malloca_s(char, 10 + strlen(asset));
	mrgstr("Loading: ", asset, buffer);
	SetState(buffer);
	freea_s(buffer);
}

void Plutonium::AssetLoader::SetStateLoadingInternal(const char * asset)
{
	lockState.lock();

	char *split[2] = { malloca_s(char, FILENAME_MAX), malloca_s(char, FILENAME_MAX) };
	size_t cnt = spltstr(state, '>', split, 0);
	char *buffer = malloca_s(char, strlen(split[0]) + strlen(asset) + 4);
	const char *args[] = { split[0], "> ", asset };
	mrgstr(args, 3, buffer);
	lockState.unlock();

	SetState(buffer);
	freea_s(buffer);
	for (size_t i = 0; i < cnt; i++) freea_s(split[i]);
}

void Plutonium::AssetLoader::SetState(const char * value)
{
	/* Safely set the loading state. */
	lockState.lock();
	free_s(state);
	state = heapstr(value);
	lockState.unlock();
}

void Plutonium::AssetLoader::IoThreadInit(const TickThread *, EventArgs)
{
	threadID = _CrtGetCurrentThreadId();
}

bool Plutonium::AssetLoader::OnIoThread(void)
{
	return _CrtGetCurrentThreadId() == threadID;
}

int32 Plutonium::AssetLoader::GetTextureIdx(const char * path)
{
	/* Attempt to find the requested texture or return -1 when not found. */
	for (size_t i = 0; i < loadedTextures.size(); i++)
	{
		if (!strcmp(loadedTextures.at(i)->Path, path)) return static_cast<int32>(i);
	}

	return -1;
}

int32 Plutonium::AssetLoader::GetModelIdx(const char * path)
{
	/* Attempt to find the requested texture or return -1 when not found. */
	for (size_t i = 0; i < loadedModels.size(); i++)
	{
		if (!strcmp(loadedModels.at(i)->Path, path)) return static_cast<int32>(i);
	}

	return -1;
}

void Plutonium::AssetLoader::LoadTextureInternal(TextureLoadInfo *info, bool updateState)
{
	/* Update state is requested. */
	if (updateState) SetStateLoadingInternal(info->Names->GetFileNameWithoutExtension());

	/* Load texture. */
	const char *fullPath = CreateFullPath(info->Names->GetFilePath());
	AssetInfo<Texture> *result = new AssetInfo<Texture>(info, Texture::FromFile(fullPath, wnd, info->Options));
	freea_s(fullPath);

	/* Push to loaded list. */
	lockTex.lock();
	loadedTextures.push_back(result);
	lockTex.unlock();

	/* Call callback. */
	info->Callback->HandlePost(this, result->Asset);
	delete_s(info);
}

void Plutonium::AssetLoader::LoadModelInternal(AssetLoadInfo<StaticModel> *info, bool updateState)
{
	/* Update state is requested. */
	if (updateState) SetStateLoadingInternal(info->Names->GetFileNameWithoutExtension());

	/* Load model. */
	const char *fullPath = CreateFullPath(info->Names->GetFilePath());
	AssetInfo<StaticModel> *result = new AssetInfo<StaticModel>(info, StaticModel::FromFile(fullPath, this));
	freea_s(fullPath);

	/* Push to loaded list. */
	lockMod.lock();
	loadedModels.push_back(result);
	lockMod.unlock();

	/* Call callback. */
	info->Callback->HandlePost(this, result->Asset);
	delete_s(info);
}

void Plutonium::AssetLoader::TickIoTextures(const TickThread *, EventArgs)
{
	/* Lock the buffer to make sure we can safely read the size. */
	lockTex.lock();
	while (queuedTextures.size() > 0)
	{
		/* Get current texture queued for loading. */
		TextureLoadInfo *cur = queuedTextures.back();
		queuedTextures.pop();

		/* Unload the buffer to allow more additions whilst we load the texture. */
		lockTex.unlock();
		SetStateLoading(cur->Names->GetFileNameWithoutExtension());
		LoadTextureInternal(cur, false);
		lockTex.lock();
	}

	/* Make sure we unlock the buffer. */
	lockTex.unlock();
	SetStateWaiting();
}

void Plutonium::AssetLoader::TickIoModels(const TickThread *, EventArgs)
{
	/* Lock the buffer to make sure we safely read the size. */
	lockMod.lock();
	while (queuedModels.size() > 0)
	{
		/* Get current model queued for loading. */
		AssetLoadInfo<StaticModel> *cur = queuedModels.back();
		queuedModels.pop();

		/* Unload the buffer to allow more additions whilst we load the model. */
		lockMod.unlock();
		SetStateLoading(cur->Names->GetFileNameWithoutExtension());
		LoadModelInternal(cur, false);
		lockMod.lock();
	}

	/* Make sure we unlock the buffer. */
	lockMod.unlock();
	SetStateWaiting();
}