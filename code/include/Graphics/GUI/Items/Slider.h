#pragma once
#include "ProgressBar.h"

namespace Plutonium
{
	/* Defines a user changable progress bar. */
	struct Slider
		: public ProgressBar
	{
	public:
		/* Occurs when the user starts holding the holder bar. */
		EventBus<Slider, CursorHandler> DragStart;
		/* Occurs when the user stops holding the holder bar. */
		EventBus<Slider, CursorHandler> DragEnd;
		/* Occurs when the value of the holder bar color changes. */
		EventBus<Slider, ValueChangedEventArgs<Color>> HolderBarColorChanged;
		/* Occurs when the value of the holder bar image is changed or set. */
		EventBus<Slider, ValueChangedEventArgs<TextureHandler>> HolderBarImageChanged;
		/* Occurs when the size of the holder bar is changed. */
		EventBus<Slider, ValueChangedEventArgs<Vector2>> HolderBarResized;

		/* Initializes a new instance of a slider with default parameters. */
		Slider(_In_ Game *parent);
		/* Initializes a new instance of a slider. */
		Slider(_In_ Game *parent, _In_ Rectangle bounds);
		Slider(_In_ const Slider &value) = delete;
		Slider(_In_ Slider &&value) = delete;
		/* Releases the resources allocated by the slider. */
		~Slider(void);

		_Check_return_ Slider& operator =(_In_ const Slider &other) = delete;
		_Check_return_ Slider& operator =(_In_ Slider &&other) = delete;

		/* Updates the slider checking for hold and click events. */
		virtual void Update(_In_ float dt) override;
		/* Renders the slider to the screen. */
		virtual void Draw(_In_ GuiItemRenderer *renderer) override;

		/* Gets the default bounds of the slider. */
		_Check_return_ static inline Rectangle GetDefaultBounds(void)
		{
			return Rectangle(0.0f, 0.0f, 200.0f, 5.0f);
		}

		/* Gets the default size of the holder bar. */
		_Check_return_ static inline Vector2 GetDefaultHolderBarSize(void)
		{
			return Vector2(10.0f, 25.0f);
		}

		/* Gets the default color of the holder bar. */
		_Check_return_ static inline Color GetDefaultHolderBarColor(void)
		{
			return Color::Abbey();
		}

		/* Gets whether the holder bar should be drawn. */
		_Check_return_ inline bool IsHolderBarEnabled(void) const
		{
			return holderBarEnabled;
		}

		/* Gets whether this slider allows user to randomly click the bar to change the value. */
		_Check_return_ inline bool AllowsRandomClicks(void) const
		{
			return allowRandomClicks;
		}

		/* Gets the current color of the holder bar. */
		_Check_return_ inline Color GetHolderBarColor(void) const
		{
			return holderBarColor;
		}

		/* Gets the current image used with the holder bar. */
		_Check_return_ inline TextureHandler GetHolderBarImage(void) const
		{
			return holderBar;
		}

		/* Gets the current size of the holder bar. */
		_Check_return_ inline Vector2 GetHolderBarSize(void) const
		{
			return holderBarBounds.Size;
		}

		/* Set the state of the holder bar, either enabled or disabled. */
		void SetHolderBarState(_In_ bool enabled);
		/* Sets whether the user is allowed to change the value by randomly clicking on the bar. */
		void SetRandomClicks(_In_ bool allow);
		/* Sets the color of the holder bar. */
		void SetHolderBarColor(_In_ Color color);
		/* Sets the image used to render to holder bar. */
		void SetHolderBarImage(_In_ TextureHandler image);
		/* Sets the size of the holder bar. */
		void SetHolderBarSize(_In_ Vector2 value);

	protected:
		/* Renderes the Slider to the renderer, used for internal item skipping. */
		void RenderSlider(_In_ GuiItemRenderer *renderer);

	private:
		bool holderBarEnabled, allowRandomClicks, dragInvoked;
		Color holderBarColor;
		TextureHandler holderBar;
		Rectangle holderBarBounds;
		Buffer *holderBarMesh;

		void OnRandomClick(const GuiItem*, CursorHandler cursor);
		void UpdateHolderBarPos(void);
		void UpdateHolderBarMesh(void);
	};
}