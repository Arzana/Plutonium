#pragma once
#include "Components\GameComponent.h"
#include "Graphics\GUI\Containers\GUIWindow.h"
#include "Graphics\GUI\Items\Slider.h"
#include "Graphics\GUI\Items\TextBox.h"

namespace Plutonium
{
	class GuiItemRenderer;

	/* Defines a manager object for GuiItems. */
	class Menu
		: public GameComponent, protected Container
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

		/* Gets the center width of the screen. */
		_Check_return_ inline int32 GetScreenWidthCenter(void) const
		{
			return GetScreenWidth() >> 1;
		}
		/* Gets the center height of the screen. */
		_Check_return_ inline int32 GetScreenHeightCenter(void) const
		{
			return GetScreenHeight() >> 1;
		}

		/* Hides the menu, disabling it and rendering it invisible. */
		void Hide(void);
		/* Shows the menu, enabling it and making is visible. */
		void Show(void);

		/* Gets the width of the screen. */
		_Check_return_ int32 GetScreenWidth(void) const;
		/* Gets the height of the screen. */
		_Check_return_ int32 GetScreenHeight(void) const;
		/* Check if any GuiItem has focus in this menu. */
		_Check_return_ bool HasFocus(void) const;

	protected:
		/* Gets the font specified as the default font. */
		_Check_return_ const Font* GetDefaultFont(void) const;
		/* Gets a named font from the loaded fonts. */
		_Check_return_ const Font* GetFont(_In_ const char *name) const;
		/* Gets a named texture from the loaded textures. */
		_Check_return_ TextureHandler GetTexture(_In_ const char *name) const;

		/* Adds a basic GuiItem to the menu. */
		_Check_return_ GuiItem* AddGuiItem(void);
		/* Adds a label to the menu, if no font is specified the default font will be used. */
		_Check_return_ Label* AddLabel(_In_opt_ const Font *font = nullptr);
		/* Adds a button to the menu, if no font is specified the default font will be used. */
		_Check_return_ Button* AddButton(_In_opt_ const Font *font = nullptr);
		/* Adds a progress bar to the menu. */
		_Check_return_ ProgressBar* AddProgressBar(void);
		/* Adds a slider to the menu. */
		_Check_return_ Slider* AddSlider(void);
		/* Adds a text box to the menu. */
		_Check_return_ TextBox* AddTextBox(_In_opt_ const Font *font = nullptr);
		/* Adds a window to the menu. */
		_Check_return_ GUIWindow* AddWindow(_In_opt_ const Font *font = nullptr);

		/* Sets the default font to a specified font. */
		void SetDefaultFont(_In_ const char *path, _In_ float size);
		/* Loads a specified font. */
		void LoadFont(_In_ const char *path, _In_ float size);
		/* Loads a specified texture. */
		void LoadTexture(_In_ const char *path, _In_opt_ const TextureCreationOptions * config = &TextureCreationOptions::Default2D);

		/* Adds a debug string to the draw queue, this should only be used for debugging purposes and will not work on release mode! */
		void DrawString(_In_ Vector2 position, _In_ const char *text, _In_opt_ Color color = Color::White());

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
		std::vector<const Font*> loadedFonts;
		std::vector<TextureHandler> loadedTextures;
		int32 defaultFontIdx;
		std::atomic_int loadCnt, loadTarget;
		std::mutex loadLock;
		bool visible, callCreate;
		GuiItemRenderer *renderer;

#if defined (DEBUG)
		Buffer *stringVbo;
#endif

		void CheckIfLoadingDone(void);
		void CheckFont(const Font *font) const;
		void OnTextBoxGainFocus(const GuiItem *txt, EventArgs);
		void OnWindowClosed(const GUIWindow *wnd, EventArgs);
	};
}