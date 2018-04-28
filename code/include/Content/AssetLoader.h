#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include "Core\Events\EventSubscriber.h"
#include "Graphics\Texture.h"
#include "Graphics\Models\StaticModel.h"
#include "Graphics\Native\Window.h"
#include "Streams\FileReader.h"

namespace Plutonium
{
	struct TickThread;
	struct EventArgs;

	/* Defines a multi-threaded asset loader. */
	struct AssetLoader
	{
	public:
		/* Initializes a new instance of a asset loader with a specified root directory. */
		AssetLoader(_In_ WindowHandler wnd, _In_opt_ const char *root = "./assets/");
		AssetLoader(_In_ const AssetLoader &value) = delete;
		AssetLoader(_In_ AssetLoader &&value) = delete;
		/* Releases the resources allocated by the asset loader. */
		~AssetLoader(void) noexcept;

		_Check_return_ AssetLoader& operator =(_In_ const AssetLoader &other) = delete;
		_Check_return_ AssetLoader& operator =(_In_ AssetLoader &&other) = delete;

		/* Gets the root directory of this asset loader. */
		_Check_return_ inline const char* GetRoot(void) const
		{
			return root;
		}
		/* Gets OpenGL context associated with this loader. */
		_Check_return_ inline WindowHandler GetWindow(void) const
		{
			return wnd;
		}

		/* gets the current state of the asset loader (requires free!). */
		_Check_return_ const char* GetState(void) const;
		/* Resets the root directory of this asset loader. */
		void SetRoot(_In_ const char *root);
		/* Removes a refrence to a specified asset. */
		_Check_return_ bool Unload(_In_ const char *path);

		/* Loads a specified texture, calls the callback after completion. */
		void LoadTexture(_In_ const char *path, _In_ EventSubscriber<AssetLoader, Texture*> *callback, _In_opt_ bool keep = false, _In_opt_ TextureCreationOptions *config = nullptr);
		/* Loads a specified model, calls the callback after completion. */
		void LoadModel(_In_ const char *path, _In_ EventSubscriber<AssetLoader, StaticModel*> *callback, _In_opt_ bool keep = false);

		/* Loads a specified texture and returns it. */
		_Check_return_ Texture* LoadTexture(_In_ const char *path, _In_opt_ bool keep = false, _In_opt_ TextureCreationOptions *config = nullptr);
		/* Loads a specified model and returns it. */
		_Check_return_ StaticModel* LoadModel(_In_ const char *path, _In_opt_ bool keep = false);

	private:

		template <typename _Ty>
		struct AssetLoadInfo
		{
			FileReader *Names;
			bool Keep;
			EventSubscriber<AssetLoader, _Ty*> *Callback;

			AssetLoadInfo(void)
				: Names(nullptr), Keep(false)
			{}

			AssetLoadInfo(FileReader *fr, bool keep, EventSubscriber<AssetLoader, _Ty*> *callback)
				: Names(fr), Keep(keep), Callback(callback)
			{}

			~AssetLoadInfo(void)
			{
				delete_s(Names);
			}
		};

		struct TextureLoadInfo
			: AssetLoadInfo<Texture>
		{
			TextureCreationOptions *Options;

			TextureLoadInfo(void)
				: AssetLoadInfo(), Options(nullptr)
			{}

			TextureLoadInfo(FileReader *fr, bool keep, EventSubscriber<AssetLoader, Texture*> *callback, TextureCreationOptions *opt)
				: AssetLoadInfo(fr, keep, callback), Options(opt)
			{}
		};

		template <typename _Ty>
		struct AssetInfo
		{
			const char *Path;
			bool Keep;
			int32 RefCnt;
			_Ty *Asset;

			AssetInfo(void)
				: Path(nullptr), keep(false), RefCnt(0), Asset(nullptr)
			{}

			AssetInfo(AssetLoadInfo<_Ty> *info, _Ty *asset)
				: Path(heapstr(info->Names->GetFilePath())), Keep(info->Keep), RefCnt(1), Asset(asset)
			{}

			~AssetInfo(void)
			{
				free_s(Path);
				delete_s(Asset);
			}
		};

		WindowHandler wnd;
		TickThread *ioThread;
		std::mutex lockRoot;
		mutable std::mutex lockState;
		std::mutex lockTex;
		std::mutex lockMod;
		int64 threadID;

		const char *root;
		size_t rootLen;
		const char *state;

		std::queue<TextureLoadInfo*> queuedTextures;
		std::queue<AssetLoadInfo<StaticModel>*> queuedModels;

		std::vector<AssetInfo<Texture>*> loadedTextures;
		std::vector<AssetInfo<StaticModel>*> loadedModels;

		const char* CreateFullPath(const char *fpath);
		void SetStateWaiting(void);
		void SetStateLoading(const char *asset);
		void SetStateLoadingInternal(const char *asset);
		void SetState(const char *value);
		void IoThreadInit(const TickThread*, EventArgs);
		bool OnIoThread(void);

		int32 GetTextureIdx(const char *path);
		int32 GetModelIdx(const char *path);

		void LoadTextureInternal(TextureLoadInfo *info, bool updateState);
		void LoadModelInternal(AssetLoadInfo<StaticModel> *info, bool updateState);

		void TickIoTextures(const TickThread*, EventArgs);
		void TickIoModels(const TickThread*, EventArgs);
	};
}