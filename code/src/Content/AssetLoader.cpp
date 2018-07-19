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
	ioThread->Initialize.Add([&](TickThreadHandler, EventArgs) { threadID = _CrtGetCurrentThreadId(); });

	/*
	Add specified loader methods to the tick queue.
	Make sure the fonts have priority because they will most likely be used in the load screen.
	*/
	ioThread->Tick.Add(this, &AssetLoader::TickIoFonts);
	ioThread->Tick.Add(this, &AssetLoader::TickIoTextures);
	ioThread->Tick.Add(this, &AssetLoader::TickIoSModels);
	ioThread->Tick.Add(this, &AssetLoader::TickIoDModels);

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

	/* Unload all stored static models. */
	while (loadedSModels.size())
	{
		AssetInfo<StaticModel> *cur = loadedSModels.back();
		LOG_WAR_IF(cur->RefCnt > 0, "Active refrence to model '%s' is removed!", cur->Asset->GetName());
		delete_s(cur);
		loadedSModels.pop_back();
	}

	/* Unload all stored dynamic models. */
	while (loadedDModels.size())
	{
		AssetInfo<DynamicModel> *cur = loadedDModels.back();
		LOG_WAR_IF(cur->RefCnt > 0, "Active refrence to model '%s' is removed!", cur->Asset->GetName());
		delete_s(cur);
		loadedDModels.pop_back();
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

bool Plutonium::AssetLoader::Unload(TextureHandler texture)
{
	/* Check if texture was loaded from file. */
	ASSERT_IF(!texture, "Input texture cannot be null!");
	if (!texture->path) return false;

	/* Searches the buffer for the texture. */
	lockTex.lock();
	int32 idx = GetTextureIdx(texture->path, texture->config);
	if (idx != -1)
	{
		/* Check for corrupt unloading. */
		AssetInfo<Texture> *cur = loadedTextures.at(idx);
		LOG_WAR_IF(cur->RefCnt <= 0, "Attempting to unload unrefrenced texture!");

		/* Remove refrence to texture and checks if it needs to be deleted. */
		if (cur->RemoveRef())
		{
			/* Delete texture and remove it from the list. */
			LOG("Removing refrence to texture '%s'.", cur->Asset->GetName());
			loadedTextures.erase(loadedTextures.begin() + idx);
			lockTex.unlock();
			delete_s(cur);
			return true;
		}

		lockTex.unlock();
		return true;
	}
	lockTex.unlock();

	/* Unable to find specified texture. */
	return false;
}

bool Plutonium::AssetLoader::Unload(const StaticModel * model)
{
	/* Check if model was loaded from file. */
	ASSERT_IF(!model, "Input model cannot be null!");
	if (!model->path) return false;

	/* Searches the buffer for the model. */
	lockSMod.lock();
	int32 idx = GetSModelIdx(model->path);
	if (idx != -1)
	{
		/* Check for corrupt unloading. */
		AssetInfo<StaticModel> *cur = loadedSModels.at(idx);
		LOG_WAR_IF(cur->RefCnt <= 0, "Attempting to unload unrefrenced model!");

		/* Remove refrence to model and check if it needs to be deleted. */
		if (cur->RemoveRef())
		{
			/* Delete model and remove it from the list. */
			LOG("Removing refrence to model '%s'.", cur->Asset->GetName());
			loadedSModels.erase(loadedSModels.begin() + idx);
			lockSMod.unlock();
			delete_s(cur);
			return true;
		}

		lockSMod.unlock();
		return true;
	}
	lockSMod.unlock();

	/* Unable to find specified model. */
	return false;
}

bool Plutonium::AssetLoader::Unload(const DynamicModel * model)
{
	/* Check if model was loaded from file. */
	ASSERT_IF(!model, "Input model cannot be null!");
	if (!model->path) return false;

	/* Searches the buffer for the model. */
	lockDMod.lock();
	int32 idx = GetDModelIdx(model->path);
	if (idx != -1)
	{
		/* Check for corrupt unloading. */
		AssetInfo<DynamicModel> *cur = loadedDModels.at(idx);
		LOG_WAR_IF(cur->RefCnt <= 0, "Attempting to unload unrefrenced model!");

		/* Remove refrence to model and check if it needs to be deleted. */
		if (cur->RemoveRef())
		{
			/* Delete model and remove it from the list. */
			LOG("Removing refrence to model '%s'.", cur->Asset->GetName());
			loadedDModels.erase(loadedDModels.begin() + idx);
			lockDMod.unlock();
			delete_s(cur);
			return true;
		}

		lockDMod.unlock();
		return true;
	}
	lockDMod.unlock();

	/* Unable to find specified model. */
	return false;
}

bool Plutonium::AssetLoader::Unload(const Font * font)
{
	/* Check if font was loaded from file. */
	ASSERT_IF(!font, "Input font cannot be null!");
	if (!font->path) return false;

	/* Searches the buffer for the font. */
	lockFont.lock();
	int32 idx = GetFontIdx(font->path);
	if (idx != -1)
	{
		/* Check for corrupt unloading. */
		AssetInfo<Font> *cur = loadedFonts.at(idx);
		LOG_WAR_IF(cur->RefCnt <= 0, "Attempting to unload unrefrenced font!");

		/* Remove refrence to font and check if it needs to be deleted. */
		if (cur->RemoveRef())
		{
			/* Delete font and remove it from the list. */
			LOG("Removing refrence to font '%s'.", cur->Asset->GetName());
			loadedFonts.erase(loadedFonts.begin() + idx);
			lockFont.unlock();
			delete_s(cur);
			return true;
		}

		lockFont.unlock();
		return true;
	}
	lockFont.unlock();

	/* Unable to find specified font. */
	return false;
}

void Plutonium::AssetLoader::LoadTexture(const char * path, EventSubscriber<AssetLoader, Texture*> &callback, bool keep, const TextureCreationOptions * config)
{
	lockTex.lock();

	int32 idx = GetTextureIdx(path, *config);
	if (idx != -1)
	{
		/* If texture is already loaded return it and increase it's refrence count. */
		AssetInfo<Texture> *cur = loadedTextures.at(idx);
		++cur->RefCnt;
		callback.HandlePost(this, cur->Asset);
		lockTex.unlock();
	}
	else
	{
		/* Check if asset is already in the load queue. */
		for (size_t i = 0; i < queuedTextures.size(); i++)
		{
			TextureLoadInfo *cur = queuedTextures.at(i);
			if (eqlstr(cur->Names->GetFilePath(), path) && *cur->Options == *config)
			{
				/* Add callback to the list if asset found and increase ref count. */
				++cur->RefCnt;
				cur->Callbacks.push_back(std::move(callback));
				lockTex.unlock();
				return;
			}
		}

		/* We have to load the texture so create an infomation struct. */
		TextureLoadInfo *info = new TextureLoadInfo(new FileReader(path, true), keep, callback, config);

		/* Make sure we actually load the texture on the IO thread. */
		if (OnIoThread())
		{
			lockTex.unlock();
			LoadTextureInternal(info);
		}
		else
		{
			queuedTextures.push_back(info);
			lockTex.unlock();
		}
	}
}

void Plutonium::AssetLoader::LoadTexture(const char * paths[6], EventSubscriber<AssetLoader, Texture*>& callback, bool keep, const TextureCreationOptions * config)
{
	lockTex.lock();

	int32 idx = GetTextureIdx(paths[0], *config);
	if (idx != -1)
	{
		/* If texture is already loaded return it and increase it's refrence count. */
		AssetInfo<Texture> *cur = loadedTextures.at(idx);
		++cur->RefCnt;
		callback.HandlePost(this, cur->Asset);
		lockTex.unlock();
	}
	else
	{
		/* Check if asset is already in the load queue. */
		for (size_t i = 0; i < queuedTextures.size(); i++)
		{
			TextureLoadInfo *cur = queuedTextures.at(i);
			if (eqlstr(cur->Names->GetFilePath(), paths[0]) && *cur->Options == *config)
			{
				/* Add callback to the list if asset found and increase ref count. */
				++cur->RefCnt;
				cur->Callbacks.push_back(std::move(callback));
				lockTex.unlock();
				return;
			}
		}

		ASSERT_IF(config->Type != TextureType::TextureCube, "Invalid teture creation options for skybox!");

		/* We have to load the texture so create an infomation struct. */
		FileReader *readers = calloc_s(FileReader, Texture::CUBEMAP_TEXTURE_COUNT);
		for (size_t i = 0; i < Texture::CUBEMAP_TEXTURE_COUNT; i++) readers[i] = FileReader(paths[i], true);

		TextureLoadInfo *info = new TextureLoadInfo(readers, keep, callback, config);

		/* Make sure we actually load the texture on the IO thread. */
		if (OnIoThread())
		{
			lockTex.unlock();
			LoadSkyboxInternal(info);
		}
		else
		{
			queuedTextures.push_back(info);
			lockTex.unlock();
		}
	}
}

void Plutonium::AssetLoader::LoadModel(const char * path, EventSubscriber<AssetLoader, StaticModel*> &callback, bool keep)
{
	lockSMod.lock();

	int32 idx = GetSModelIdx(path);
	if (idx != -1)
	{
		/* If model is already loaded return it and increase it's refrence count. */
		AssetInfo<StaticModel> *cur = loadedSModels.at(idx);
		++cur->RefCnt;
		callback.HandlePost(this, cur->Asset);
		lockSMod.unlock();
	}
	else
	{
		/* Check if asset is already in the load queue. */
		for (size_t i = 0; i < queuedSModels.size(); i++)
		{
			AssetLoadInfo<StaticModel> *cur = queuedSModels.at(i);
			if (!strcmp(cur->Names->GetFilePath(), path))
			{
				/* Add callback to the list if asset found and increase ref count. */
				++cur->RefCnt;
				cur->Callbacks.push_back(std::move(callback));
				lockSMod.unlock();
				return;
			}
		}

		/* We have to load the model so create an information struct. */
		AssetLoadInfo<StaticModel> *info = new AssetLoadInfo<StaticModel>(new FileReader(path, true), keep, callback);

		/* Make sure we actually load the model on the IO thread. */
		if (OnIoThread())
		{
			lockSMod.unlock();
			LoadSModelInternal(info);
		}
		else
		{
			queuedSModels.push_back(info);
			lockSMod.unlock();
		}
	}
}

void Plutonium::AssetLoader::LoadModel(const char * path, EventSubscriber<AssetLoader, DynamicModel*>& callback, bool keep, const char * texture)
{
	lockDMod.lock();

	int32 idx = GetDModelIdx(path);
	if (idx != -1)
	{
		/* If model is already loaded return it and increase it's refrence count. */
		AssetInfo<DynamicModel> *cur = loadedDModels.at(idx);
		++cur->RefCnt;
		callback.HandlePost(this, cur->Asset);
		lockDMod.unlock();
	}
	else
	{
		/* Check if asset is already in the load queue. */
		for (size_t i = 0; i < queuedDModels.size(); i++)
		{
			DynamicModelLoadInfo *cur = queuedDModels.at(i);
			if (!strcmp(cur->Names->GetFilePath(), path))
			{
				/* Add callback to the list if asset found and increase ref count. */
				++cur->RefCnt;
				cur->Callbacks.push_back(std::move(callback));
				lockDMod.unlock();
				return;
			}
		}

		/* We have to load the model so create an information struct. */
		DynamicModelLoadInfo *info = new DynamicModelLoadInfo(new FileReader(path, true), keep, callback, texture);

		/* Make sure we actually load the model on the IO thread. */
		if (OnIoThread())
		{
			lockDMod.unlock();
			LoadDModelInternal(info);
		}
		else
		{
			queuedDModels.push_back(info);
			lockDMod.unlock();
		}
	}
}

void Plutonium::AssetLoader::LoadFont(const char * path, EventSubscriber<AssetLoader, Font*>& callback, float scale, bool keep)
{
	lockFont.lock();

	int32 idx = GetFontIdx(path);
	if (idx != -1)
	{
		/* If font is already loaded return it and increase it's refrence count. */
		AssetInfo<Font> *cur = loadedFonts.at(idx);
		++cur->RefCnt;
		callback.HandlePost(this, cur->Asset);
		lockFont.unlock();
	}
	else
	{
		/* Check if asset is already in the load queue. */
		for (size_t i = 0; i < queuedFonts.size(); i++)
		{
			FontLoadInfo *cur = queuedFonts.at(i);
			if (!strcmp(cur->Names->GetFilePath(), path))
			{
				/* Add callback to the list if asset found and increase ref count. */
				++cur->RefCnt;
				cur->Callbacks.push_back(std::move(callback));
				lockFont.unlock();
				return;
			}
		}

		/* We have to load the model so create an information struct. */
		FontLoadInfo *info = new FontLoadInfo(new FileReader(path, true), keep, callback, scale);

		/* Make sure we actually load the model on the IO thread. */
		if (OnIoThread())
		{
			lockFont.unlock();
			LoadFontInternal(info);
		}
		else
		{
			queuedFonts.push_back(info);
			lockFont.unlock();
		}
	}
}

Texture * Plutonium::AssetLoader::LoadTexture(const char * path, bool keep, const TextureCreationOptions * config)
{
	/* Create temporary storage. */
	LoadResult<Texture> result;

	/* Load the texture with the specified callback. */
	LoadTexture(path, Callback<Texture>(&result, &LoadResult<Texture>::OnLoadComplete), keep, config);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

Texture * Plutonium::AssetLoader::LoadTexture(const char * paths[6], bool keep, const TextureCreationOptions * config)
{
	/* Create temporary storage. */
	LoadResult<Texture> result;

	/* Load the texture with the specified callback. */
	LoadTexture(paths, Callback<Texture>(&result, &LoadResult<Texture>::OnLoadComplete), keep, config);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

StaticModel * Plutonium::AssetLoader::LoadModel(const char * path, bool keep)
{
	/* Create temporary storage. */
	LoadResult<StaticModel> result;

	/* Load the model with the specified callback. */
	LoadModel(path, Callback<StaticModel>(&result, &LoadResult<StaticModel>::OnLoadComplete), keep);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

DynamicModel * Plutonium::AssetLoader::LoadModel(const char * path, bool keep, const char * texture)
{
	/* Create temporary storage. */
	LoadResult<DynamicModel> result;

	/* Load the model with the specified callback. */
	LoadModel(path, Callback<DynamicModel>(&result, &LoadResult<DynamicModel>::OnLoadComplete), keep, texture);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

Font * Plutonium::AssetLoader::LoadFont(const char * path, float scale, bool keep)
{
	/* Create temporary storage. */
	LoadResult<Font> result;

	/* Load the model with the specified callback. */
	LoadFont(path, Callback<Font>(&result, &LoadResult<Font>::OnLoadComplete), scale, keep);

	/* Wait untill loading is complete and return value. */
	while (!result.loaded.load()) PuThread::Sleep(10);
	return result.value;
}

const char * Plutonium::AssetLoader::CreateFullPath(const char * fpath)
{
	lockRoot.lock();

	char *buffer = nullptr;

	/* If string already contains the root skip it. */
	if (strstr(fpath, root))
	{
		lockRoot.unlock();
		buffer = malloc_s(char, strlen(fpath) + 1);
		strcpy(buffer, fpath);
	}
	else
	{
		/* Merge the two strings into a stack path. */
		buffer = malloc_s(char, rootLen + strlen(fpath) + 1);
		mrgstr(root, fpath, buffer);
		lockRoot.unlock();
	}

	return buffer;
}

void Plutonium::AssetLoader::PushState(const char * asset)
{
	loadStack.push_back(asset);
	UpdateState();
}

void Plutonium::AssetLoader::PopState(void)
{
	loadStack.pop_back();
	UpdateState();
}

void Plutonium::AssetLoader::UpdateState(void)
{
	/* Defines prefix and seperator. */
	constexpr const char *PREFIX = "Loading: ";
	constexpr const char *SEPERATOR = "> ";
	const size_t SEPERATOR_LEN = strlen(SEPERATOR);
	const size_t PREFIX_LEN = strlen(PREFIX);

	/* Set state to waiting if empty. */
	if (loadStack.size() < 1)
	{
		SetState("Waiting...");
		return;
	}

	/* Calculate final string length. */
	size_t len = SEPERATOR_LEN * (loadStack.size() - 1);
	for (size_t i = 0; i < loadStack.size(); i++)
	{
		len += strlen(loadStack.at(i));
	}

	/* Populate buffer. */
	char *buffer = malloca_s(char, len + PREFIX_LEN + 1);
	strncpy(buffer, PREFIX, PREFIX_LEN);

	for (size_t i = 0, start = PREFIX_LEN; i < loadStack.size(); i++)
	{
		const char *cur = loadStack.at(i);
		const size_t curLen = strlen(cur);

		/* Add seperator if needed. */
		if (i > 0)
		{
			strncpy(buffer + start, SEPERATOR, SEPERATOR_LEN);
			start += SEPERATOR_LEN;
		}

		/* Add current asset. */
		strncpy(buffer + start, cur, curLen);
		start += curLen;
	}

	/* Add null terminator. */
	buffer[len + PREFIX_LEN] = '\0';

	/* Set state and free temporary buffer. */
	SetState(buffer);
	freea_s(buffer);
}

void Plutonium::AssetLoader::SetState(const char * value)
{
	/* Safely set the loading state. */
	lockState.lock();
	free_s(state);
	state = heapstr(value);
	lockState.unlock();
}

bool Plutonium::AssetLoader::OnIoThread(void)
{
	return _CrtGetCurrentThreadId() == threadID;
}

int32 Plutonium::AssetLoader::GetTextureIdx(const char * path, const TextureCreationOptions & config)
{
	/* Attempt to find the requested texture or return -1 when not found. */
	for (size_t i = 0; i < loadedTextures.size(); i++)
	{
		AssetInfo<Texture> *cur = loadedTextures.at(i);

		if (eqlstr(cur->Asset->path, path) && cur->Asset->config == config) return static_cast<int32>(i);
	}

	return -1;
}

int32 Plutonium::AssetLoader::GetSModelIdx(const char * path)
{
	/* Attempt to find the requested model or return -1 when not found. */
	for (size_t i = 0; i < loadedSModels.size(); i++)
	{
		if (eqlstr(loadedSModels.at(i)->Asset->path, path)) return static_cast<int32>(i);
	}

	return -1;
}

int32 Plutonium::AssetLoader::GetDModelIdx(const char * path)
{
	/* Attempt to find the requested model or return -1 when not found. */
	for (size_t i = 0; i < loadedDModels.size(); i++)
	{
		if (eqlstr(loadedDModels.at(i)->Asset->path, path)) return static_cast<int32>(i);
	}

	return -1;
}

int32 Plutonium::AssetLoader::GetFontIdx(const char * path)
{
	/* Attempt to find the requested font or return -1 when not found. */
	for (size_t i = 0; i < loadedFonts.size(); i++)
	{
		if (!strcmp(loadedFonts.at(i)->Asset->path, path)) return static_cast<int32>(i);
	}

	return -1;
}

void Plutonium::AssetLoader::LoadTextureInternal(TextureLoadInfo *info)
{
	/* Update state is requested. */
	PushState(info->Names->GetFileNameWithoutExtension());

	/* Load texture. */
	const char *fullPath = CreateFullPath(info->Names->GetFilePath());
	AssetInfo<Texture> *result = new AssetInfo<Texture>(info->Keep, Texture::FromFile(fullPath, wnd, info->Options), info->RefCnt);
	free_s(fullPath);

	/* Push to loaded list. */
	lockTex.lock();
	loadedTextures.push_back(result);
	lockTex.unlock();

	/* Call callback. */
	for (size_t i = 0; i < info->Callbacks.size(); i++) info->Callbacks.at(i).HandlePost(this, result->Asset);
	delete_s(info);

	PopState();
}

void Plutonium::AssetLoader::LoadSkyboxInternal(TextureLoadInfo * info)
{
	/* Update state is requested. */
	PushState(info->Names->GetFileNameWithoutExtension());

	/* Load texture. */
	const char *paths[Texture::CUBEMAP_TEXTURE_COUNT];
	for (size_t i = 0; i < Texture::CUBEMAP_TEXTURE_COUNT; i++) paths[i] = CreateFullPath(info->Names[i].GetFilePath());
	AssetInfo<Texture> *result = new AssetInfo<Texture>(info->Keep, Texture::FromFile(paths, wnd, info->Options), info->RefCnt);
	for (size_t i = 0; i < Texture::CUBEMAP_TEXTURE_COUNT; i++) free_s(paths[i]);

	/* Push to loaded list. */
	lockTex.lock();
	loadedTextures.push_back(result);
	lockTex.unlock();

	/* Call callback. */
	for (size_t i = 0; i < info->Callbacks.size(); i++) info->Callbacks.at(i).HandlePost(this, result->Asset);
	delete_s(info);

	PopState();
}

void Plutonium::AssetLoader::LoadSModelInternal(AssetLoadInfo<StaticModel> *info)
{
	/* Update state is requested. */
	PushState(info->Names->GetFileNameWithoutExtension());

	/* Load model. */
	const char *fullPath = CreateFullPath(info->Names->GetFilePath());
	AssetInfo<StaticModel> *result = new AssetInfo<StaticModel>(info->Keep, StaticModel::FromFile(fullPath, this, &progression), info->RefCnt);
	free_s(fullPath);

	/* Push to loaded list. */
	lockSMod.lock();
	loadedSModels.push_back(result);
	lockSMod.unlock();

	/* Call callback. */
	for (size_t i = 0; i < info->Callbacks.size(); i++) info->Callbacks.at(i).HandlePost(this, result->Asset);
	delete_s(info);

	PopState();
	progression.store(0.0f);
}

void Plutonium::AssetLoader::LoadDModelInternal(DynamicModelLoadInfo *info)
{
	PushState(info->Names->GetFileNameWithoutExtension());

	/* Load model. */
	const char *fullPath = CreateFullPath(info->Names->GetFilePath());
	AssetInfo<DynamicModel> *result = new AssetInfo<DynamicModel>(info->Keep, DynamicModel::FromFile(fullPath, this, info->Texture, &progression), info->RefCnt);
	free_s(fullPath);

	/* Push to loaded list. */
	lockDMod.lock();
	loadedDModels.push_back(result);
	lockDMod.unlock();

	/* Call callback. */
	for (size_t i = 0; i < info->Callbacks.size(); i++) info->Callbacks.at(i).HandlePost(this, result->Asset);
	delete_s(info);

	PopState();
}

void Plutonium::AssetLoader::LoadFontInternal(FontLoadInfo * info)
{
	PushState(info->Names->GetFileNameWithoutExtension());

	/* Load font. */
	const char *fullPath = CreateFullPath(info->Names->GetFilePath());
	AssetInfo<Font> *result = new AssetInfo<Font>(info->Keep, Font::FromFile(fullPath, info->Scale, wnd), info->RefCnt);
	free_s(fullPath);

	/* Push to loaded list. */
	lockFont.lock();
	loadedFonts.push_back(result);
	lockFont.unlock();

	/* Call callback. */
	for (size_t i = 0; i < info->Callbacks.size(); i++) info->Callbacks.at(i).HandlePost(this, result->Asset);
	delete_s(info);

	PopState();
}

void Plutonium::AssetLoader::TickIoTextures(const TickThread *, EventArgs)
{
	/* Lock the buffer to make sure we can safely read the size. */
	lockTex.lock();
	while (queuedTextures.size() > 0)
	{
		/* Get current texture queued for loading. */
		TextureLoadInfo *cur = queuedTextures.front();
		queuedTextures.pop_front();

		/* Unload the buffer to allow more additions whilst we load the texture. */
		lockTex.unlock();
		if (cur->Options && cur->Options->Type == TextureType::TextureCube) LoadSkyboxInternal(cur);
		else LoadTextureInternal(cur);
		lockTex.lock();
	}

	/* Make sure we unlock the buffer. */
	lockTex.unlock();
}

void Plutonium::AssetLoader::TickIoSModels(const TickThread *, EventArgs)
{
	/* Lock the buffer to make sure we safely read the size. */
	lockSMod.lock();
	while (queuedSModels.size() > 0)
	{
		/* Get current model queued for loading. */
		AssetLoadInfo<StaticModel> *cur = queuedSModels.front();
		queuedSModels.pop_front();

		/* Unload the buffer to allow more additions whilst we load the model. */
		lockSMod.unlock();
		LoadSModelInternal(cur);
		lockSMod.lock();
	}

	/* Make sure we unlock the buffer. */
	lockSMod.unlock();
}

void Plutonium::AssetLoader::TickIoDModels(const TickThread *, EventArgs)
{
	/* Lock the buffer to make sure we safely read the size. */
	lockDMod.lock();
	while (queuedDModels.size() > 0)
	{
		/* Get current model queued for loading. */
		DynamicModelLoadInfo *cur = queuedDModels.front();
		queuedDModels.pop_front();

		/* Unload the buffer to allow more additions whilst we load the model. */
		lockDMod.unlock();
		LoadDModelInternal(cur);
		lockDMod.lock();
	}

	/* Make sure we unlock the buffer. */
	lockDMod.unlock();
}

void Plutonium::AssetLoader::TickIoFonts(const TickThread *, EventArgs)
{
	/* Lock the buffer to make sure we safely read the size. */
	lockFont.lock();
	while (queuedFonts.size() > 0)
	{
		/* Get current font queued for loading. */
		FontLoadInfo *cur = queuedFonts.front();
		queuedFonts.pop_front();

		/* Unload the buffer to allow more additions whilst we load the font. */
		lockFont.unlock();
		LoadFontInternal(cur);
		lockFont.lock();
	}

	/* Make sure we unlock the buffer. */
	lockFont.unlock();
}