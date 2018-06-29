#pragma once
#include "Components\GameComponent.h"
#include "Graphics\GUI\Items\Label.h"

namespace Plutonium
{
	struct GuiItemRenderer;

	/* Defines a manager object for GuiItems. */
	struct Menu
		: public GameComponent
	{
	public:
		/* Initializes a new instance of a menu object. */
		Menu(_In_ Game *game);
		Menu(_In_ const Menu &value) = delete;
		Menu(_In_ Menu &&value) = delete;
		/* Releases the resources allocated by the menu. */
		~Menu(void);

		_Check_return_ Menu& operator =(_In_ const Menu &other) = delete;
		_Check_return_ Menu& operator =(_In_ Menu &&other) = delete;

		/* Gets a type specified named control present in this menu. */
		template<typename _Ty>
		_Check_return_ inline _Ty* GetTypedControl(_In_ const char *name) const
		{
			return dynamic_cast<_Ty*>(GetControl(name));
		}

		/* Gets the center width of the screen. */
		_Check_return_ inline int32 GetScreenWidthCenter(void) const
		{
			return GetScreenWidth() << 1;
		}
		/* Gets the center height of the screen. */
		_Check_return_ inline int32 GetScreenHeightCenter(void) const
		{
			return GetScreenHeight() << 1;
		}

		/* Gets a named control present in this menu. */
		_Check_return_ GuiItem* GetControl(_In_ const char *name) const;
		/* Hides the menu, disabling it and rendering it invisible. */
		void Hide(void);
		/* Shows the menu, enabling it and making is visible. */
		void Show(void);

		/* Gets the width of the screen. */
		_Check_return_ int32 GetScreenWidth(void) const;
		/* Gets the height of the screen. */
		_Check_return_ int32 GetScreenHeight(void) const;

	protected:
		/* Gets a named font from the loaded fonts. */
		_Check_return_ const Font* GetFont(_In_ const char *name) const;
		/* Gets a named texture from the loaded textures. */
		_Check_return_ TextureHandler GetTexture(_In_ const char *name) const;

		/* Adds a basic GuiItem to the menu. */
		_Check_return_ GuiItem* AddGuiItem(void);
		/* Adds a label to the menu, if no font is specified the default font will be used. */
		_Check_return_ Label* AddLabel(_In_opt_ const Font *font = nullptr);

		/* Sets the default font to a specified font. */
		void SetDefaultFont(_In_ const char *path, _In_ float size);
		/* Loads a specified font. */
		void LoadFont(_In_ const char *path, _In_ float size);
		/* Loads a specified texture. */
		void LoadTexture(_In_ const char *path, _In_opt_ const TextureCreationOptions * config = &TextureCreationOptions::Default2D);

		/* Adds a debug string to the draw queue, this should only be used for debugging purposes and will not work on release mode! */
		void DrawString(_In_ Vector2 position, _In_ const char *text, _In_opt_ Color color = Color::White);

		/* Initializes the menu, asset loading should take place here. */
		virtual void Initialize(void) override;
		/* Creates the menu components, called after the loading is completed. */
		virtual void Create(void) = 0;
		/* Updates the underlying GuiItems. */
		virtual void Update(_In_ float dt) override;
		/* Renders the underlying GuiItems. */
		virtual void Render(_In_ float dt) override;
		/* Finalizes the menu. */
		virtual void Finalize(void) override;

	private:
		std::vector<GuiItem*> controlls;
		std::vector<const Font*> loadedFonts;
		std::vector<TextureHandler> loadedTextures;
		int32 defaultFontIdx;
		std::atomic_int loadCnt, loadTarget;
		std::mutex loadLock;
		GuiItemRenderer *renderer;
		bool visible, callCreate;

#if defined (DEBUG)
		Buffer *stringVbo;
#endif

		void CheckIfLoadingDone(void);
		void CheckFont(const Font *font) const;
	};
}