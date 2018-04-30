#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include "Core\Events\EventSubscriber.h"
#include "Graphics\Native\Window.h"
#include "Streams\FileReader.h"
#include "Graphics\Texture.h"
#include "Graphics\Models\StaticModel.h"
#include "Graphics\Models\DynamicModel.h"
#include "Graphics\Text\Font.h"

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

		/* Removes a refrence to the specified texture. */
		bool Unload(_In_ const Texture *texture);
		/* Removes a refrence to the specified model. */
		bool Unload(_In_ const StaticModel *model);
		/* Removes a refrence to the specified model. */
		bool Unload(_In_ const DynamicModel *model);
		/* Removes a refrence to the specified font. */
		bool Unload(_In_ const Font *font);

		/* Loads a specified texture, calls the callback after completion. */
		void LoadTexture(_In_ const char *path, _In_ EventSubscriber<AssetLoader, Texture*> &callback, _In_opt_ bool keep = false, _In_opt_ TextureCreationOptions *config = nullptr);
		/* Loads a specified model, calls the callback after completion. */
		void LoadModel(_In_ const char *path, _In_ EventSubscriber<AssetLoader, StaticModel*> &callback, _In_opt_ bool keep = false);
		/* Loads a specified model, calls the callback after completion. */
		void LoadModel(_In_ const char *path, _In_ EventSubscriber<AssetLoader, DynamicModel*> &callback, _In_opt_ bool keep = false, _In_opt_ const char *texture = nullptr);
		/* Loads a specified font, calls the callback after completion. */
		void LoadFont(_In_ const char *path, _In_ EventSubscriber<AssetLoader, Font*> &callback, _In_ float scale, _In_opt_ bool keep = false);

		/* Loads a specified texture and returns it. */
		_Check_return_ Texture* LoadTexture(_In_ const char *path, _In_opt_ bool keep = false, _In_opt_ TextureCreationOptions *config = nullptr);
		/* Loads a specified model and returns it. */
		_Check_return_ StaticModel* LoadModel(_In_ const char *path, _In_opt_ bool keep = false);
		/* Loads a specified model and returns it. */
		_Check_return_ DynamicModel* LoadModel(_In_ const char *path, _In_opt_ bool keep = false, _In_opt_ const char *texture = nullptr);
		/* Loads a specified font and returns it. */
		_Check_return_ Font* LoadFont(_In_ const char *path, _In_ float scale, _In_opt_ bool keep = false);

	private:

		template <typename _Ty>
		struct AssetLoadInfo
		{
			FileReader *Names;
			bool Keep;
			std::vector<EventSubscriber<AssetLoader, _Ty*>> Callbacks;

			AssetLoadInfo(FileReader *fr, bool keep, EventSubscriber<AssetLoader, _Ty*> &callback)
				: Names(fr), Keep(keep)
			{
				Callbacks.push_back(std::move(callback));
			}

			~AssetLoadInfo(void)
			{
				delete_s(Names);
			}
		};

		struct TextureLoadInfo
			: AssetLoadInfo<Texture>
		{
			TextureCreationOptions *Options;

			TextureLoadInfo(FileReader *fr, bool keep, EventSubscriber<AssetLoader, Texture*> &callback, TextureCreationOptions *opt)
				: AssetLoadInfo(fr, keep, callback), Options(opt)
			{}
		};

		struct DynamicModelLoadInfo
			: AssetLoadInfo<DynamicModel>
		{
			const char *Texture;

			DynamicModelLoadInfo(FileReader *fr, bool keep, EventSubscriber<AssetLoader, DynamicModel*> &callback, const char *texture)
				: AssetLoadInfo(fr, keep, callback), Texture(texture)
			{}
		};

		struct FontLoadInfo
			:AssetLoadInfo<Font>
		{
			float Scale;

			FontLoadInfo(FileReader *fr, bool keep, EventSubscriber<AssetLoader, Font*> &callback, float scale)
				: AssetLoadInfo(fr, keep, callback), Scale(scale)
			{}
		};

		template <typename _Ty>
		struct AssetInfo
		{
			bool Keep;
			int32 RefCnt;
			_Ty *Asset;

			AssetInfo(bool keep, _Ty *asset)
				: Keep(keep), RefCnt(1), Asset(asset)
			{}

			~AssetInfo(void)
			{
				delete_s(Asset);
			}

			bool RemoveRef(void)
			{
				return (--RefCnt) <= 0 && !Keep;
			}
		};

		WindowHandler wnd;
		TickThread *ioThread;
		int64 threadID;
		std::mutex lockRoot;
		mutable std::mutex lockState;
		std::mutex lockTex, lockSMod, lockDMod, lockFont;

		const char *root;
		size_t rootLen;
		const char *state;

		std::deque<TextureLoadInfo*> queuedTextures;
		std::deque<AssetLoadInfo<StaticModel>*> queuedSModels;
		std::deque<DynamicModelLoadInfo*> queuedDModels;
		std::deque<FontLoadInfo*> queuedFonts;

		std::vector<AssetInfo<Texture>*> loadedTextures;
		std::vector<AssetInfo<StaticModel>*> loadedSModels;
		std::vector<AssetInfo<DynamicModel>*> loadedDModels;
		std::vector<AssetInfo<Font>*> loadedFonts;

		const char* CreateFullPath(const char *fpath);
		void SetStateWaiting(void);
		void SetStateLoading(const char *asset);
		void SetStateLoadingInternal(const char *asset);
		void SetState(const char *value);
		void IoThreadInit(const TickThread*, EventArgs);
		bool OnIoThread(void);

		int32 GetTextureIdx(const char *path);
		int32 GetSModelIdx(const char *path);
		int32 GetDModelIdx(const char *path);
		int32 GetFontIdx(const char *path);

		void LoadTextureInternal(TextureLoadInfo *info, bool updateState);
		void LoadSModelInternal(AssetLoadInfo<StaticModel> *info, bool updateState);
		void LoadDModelInternal(DynamicModelLoadInfo *info, bool updateState);
		void LoadFontInternal(FontLoadInfo *info, bool updateState);

		void TickIoTextures(const TickThread*, EventArgs);
		void TickIoSModels(const TickThread*, EventArgs);
		void TickIoDModels(const TickThread*, EventArgs);
		void TickIoFonts(const TickThread*, EventArgs);
	};

	/* Defines the generic callback method. */
	template <typename _Ty>
	using Callback = EventSubscriber<AssetLoader, _Ty*>;
}