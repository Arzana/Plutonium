#pragma once
#include "Core\Events\ValueChangedEventArgs.h"
#include "Graphics\GUI\GuiItemRenderer.h"
#include "Core\Events\EventBus.h"
#include "Anchors.h"
#include "Game.h"

namespace Plutonium
{
	/* Defines the absolute base object for all GuiItems. */
	struct GuiItem
	{
	public:
		/* Occurs when the BackColor value is changed. */
		EventBus<GuiItem, ValueChangedEventArgs<Color>> BackColorChanged;
		/* Occurs when the BackgroundImage is set or changed. */
		EventBus<GuiItem, ValueChangedEventArgs<TextureHandler>> BackgroundImageChanged;
		/* Occurs when the GuiItem is clicked with any button. */
		EventBus<GuiItem, CursorHandler> Clicked;
		/* Occurs before the deletion of the base GuiItem. */
		EventBus<GuiItem, EventArgs> Finalized;
		/* Occurs when the Focusable indicator is changed. */
		EventBus<GuiItem, ValueChangedEventArgs<bool>> FocusableChanged;
		/* Occurs when the FocusedImage is set or changed. */
		EventBus<GuiItem, ValueChangedEventArgs<TextureHandler>> FocusedImageChanged;
		/* Occurs when the GuiItem gains focus. */
		EventBus<GuiItem, EventArgs> GainedFocus;
		/* Occurs when the cursor pointer rests on the GuiItem. */
		EventBus<GuiItem, CursorHandler> Hover;
		/* Occurs when the cursor pointer enters the GuiItem's bounds. */
		EventBus<GuiItem, CursorHandler> HoverEnter;
		/* Occurs when the cursor pointer leaves the GuiItem's bounds. */
		EventBus<GuiItem, CursorHandler> HoverLeave;
		/* Occurs when the GuiItem loses focus. */
		EventBus<GuiItem, EventArgs> LostFocus;
		/* Occurs when the position of the GuiItem is changed. */
		EventBus<GuiItem, ValueChangedEventArgs<Vector2>> Moved;
		/* Occurs when the name of the GuiItem is set or changed. */
		EventBus<GuiItem, ValueChangedEventArgs<const char*>> NameChanged;
		/* Occurs when the GuiItem is resized. */
		EventBus<GuiItem, ValueChangedEventArgs<Vector2>> Resized;
		/* Occurs when the GuiItem is enabled or disabled. */
		EventBus<GuiItem, ValueChangedEventArgs<bool>> StateChanged;
		/* Occurs when the GuiItem is shown or hiden. */
		EventBus<GuiItem, ValueChangedEventArgs<bool>> VisibilityChanged;

		/* Initializes a new instance of a base GuiItem with default settings. */
		GuiItem(_In_ Game *parent);
		/* Initializes a new instance of a base GuiItem with a specified position and size. */
		GuiItem(_In_ Game *parent, _In_ Rectangle bounds);
		GuiItem(_In_ const GuiItem &value) = delete;
		GuiItem(_In_ GuiItem &&value) = delete;
		/* Releases the resources allocated by the GuiItem. */
		~GuiItem(void);

		_Check_return_ GuiItem& operator =(_In_ const GuiItem &other) = delete;
		_Check_return_ GuiItem& operator =(_In_ GuiItem &&other) = delete;

		/* Simulated a cursor click event. */
		void PerformClick(void);
		/* Updates the GuiItem, checking if any event are occuring. */
		virtual void Update(_In_ float dt);
		/* Renders the GuiItem to the screen. */
		virtual void Draw(_In_ GuiItemRenderer *renderer);
		/* 
		Moves the GuiItem to a specified relative position.
		The anchor will have prefrence over the specified positional components.
		*/
		void MoveRelative(_In_ Anchors anchor, _In_opt_ float x = 0.0f, _In_opt_ float y = 0.0f);
		/* Enables the GuiItem and makes it visible. */
		void Show(void);
		/* Disables the GuiItem and makes it hiden. */
		void Hide(void);

		/* Gets the current value of the anchor. */
		_Check_return_ inline Anchors GetAnchor(void) const
		{
			return anchor;
		}

		/* Gets the current value of the background color. */
		_Check_return_ inline Color GetBackColor(void) const
		{
			return backColor;
		}

		/* Gets the current background image (nullptr is none is set). */
		_Check_return_ inline TextureHandler GetBackgroundImage(void) const
		{
			return background;
		}

		/* Gets the bounds of the GuiItem. */
		_Check_return_ inline Rectangle GetBounds(void) const
		{
			return bounds;
		}

		/* Gets the default value for the background color. */
		_Check_return_ static inline Color GetDefaultBackColor(void)
		{
			return Color::WhiteSmoke();
		}

		/* Gets the default value for the GuiItem bounds. */
		_Check_return_ static inline Rectangle GetDefaultBounds(void)
		{
			return Rectangle(0.0f, 0.0f, 100.0f, 50.0f);
		}

		/* Gets the default value for the rounding factor. */
		_Check_return_ static inline float GetDefaultRoundingFactor(void)
		{
			return 10.0f;
		}

		/* Gets whether the GuiItem is currently enabled. */
		_Check_return_ inline bool IsEnabled(void) const
		{
			return enabled;
		}

		/* Gets the current height of the bounds as an integer value. */
		_Check_return_ inline int32 GetHeight(void) const
		{
			return static_cast<int32>(bounds.GetHeight());
		}

		/* Gets the assigned name of the GuiItem. */
		_Check_return_ inline const char* GetName(void) const
		{
			return name;
		}

		/* Gets the current position of the GuiItem. */
		_Check_return_ inline Vector2 GetPosition(void) const
		{
			return bounds.Position;
		}

		/* Gets the current size of the GuiItem. */
		_Check_return_ inline Vector2 GetSize(void) const
		{
			return bounds.Size;
		}

		/* Gets whether the GuiItem is currently visible. */
		_Check_return_ inline bool IsVisible(void) const
		{
			return visible;
		}

		/* Gets the current widht of the bounds as an integer value. */
		_Check_return_ inline int32 GetWidth(void) const
		{
			return static_cast<int32>(bounds.GetWidth());
		}

		/* Gets the current horizontal position of the GuiItem. */
		_Check_return_ inline float GetX(void) const
		{
			return bounds.Position.X;
		}

		/* Gets the current vertical position of the GuiItem. */
		_Check_return_ inline float GetY(void) const
		{
			return bounds.Position.Y;
		}

		/* Gets whether the GuiItem can be focused. */
		_Check_return_ inline bool IsFocusable(void) const
		{
			return focusable;
		}

		/* Gets whether the GuiItem is currently focused. */
		_Check_return_ inline bool IsFocused(void) const
		{
			return focused;
		}

		/* Gets the rounding factor. */
		_Check_return_ inline float GetRoundingFactor(void) const
		{
			return roundingFactor;
		}

		/* Gets the current offset from the defined anchor point. */
		_Check_return_ inline Vector2 GetOffsetFromAnchor(void) const
		{
			return offsetFromAnchorPoint;
		}

		/* Sets the anchor to the specified value, making sure the GuiItem always stays at the desired position if the parent or GuiItem resizes. */
		virtual void SetAnchors(_In_ Anchors value, _In_opt_ float xOffset = 0.0f, _In_opt_ float yOffset = 0.0f);
		/* Sets the color of the background to a new solid color, or (when a background image is set) changes the color filter of the background image. */
		virtual void SetBackColor(_In_ Color color);
		/* Sets the background image for this GuiItem replacing the solid color background. */
		virtual void SetBackgroundImage(_In_ TextureHandler image);
		/* Sets the focused background image for this GuiItem replacing the solid color background. */
		virtual void SetFocusedBackgroundImage(_In_ TextureHandler image);
		/* Sets the bounds of the GuiItem, possibly moving it and resizing it. */
		virtual void SetBounds(_In_ Rectangle bounds);
		/* Sets whether the GuiItem is enabled or disabled. */
		virtual void SetState(_In_ bool enabled);
		/* Sets the height of the GuiItem, resizing it. */
		virtual void SetHeight(_In_ int32 height);
		/* Sets the name indentifier of the GuiItem. */
		virtual void SetName(_In_ const char *name);
		/* Sets the position of the GuiItem. */
		virtual void SetPosition(_In_ Vector2 position);
		/* Sets the size of the GuiItem. */
		virtual void SetSize(_In_ Vector2 size);
		/* Sets whether the GuiItem is visible or hiden. */
		virtual void SetVisibility(_In_ bool visibility);
		/* Sets the width of the GuiItem, resizing it. */
		virtual void SetWidth(_In_ int32 width);
		/* Sets the horizontal position of the GuiItem, moving it. */
		virtual void SetX(_In_ float x);
		/* Sets the vertical position of the GuiItem, moving it. */
		virtual void SetY(_In_ float y);
		/* Sets whether the GuiItem can be focused. */
		virtual void SetFocusable(_In_ bool value);
		/* Sets the rounding factor used to give the GuiItem background rounded edges. */
		virtual void SetRoundingFactor(_In_ float value);

	protected:
		/* Suppresses all the refresh calls to this GuiItem until enabled again. */
		bool suppressRefresh;
		/* Suppresses the next update call to this GuiItem, resetting it afterwards. */
		bool suppressUpdate;
		/* Suppresses the next render call to this GuiItem, resetting it afterwards. */
		bool suppressRender;
		/* The game associated with the GuiItem. */
		Game *game;

		/* Renders the GuiItem to the renderer, use for internal item skipping. */
		void RenderGuiItem(_In_ GuiItemRenderer *renderer);
		/* Gets the required size of the GuiItem at any time, max of background or focus image. */
		_Check_return_ virtual Vector2 GetMinSize(void) const;
		/* This function can be called to give the GuiItem focus or have it lose focus. */
		void ApplyFocus(bool focused);

		/* Gets whether the cursor is currently hovering over the GuiItem. */
		_Check_return_ inline bool IsMouseOver(void) const
		{
			return over;
		}

		/* Gets whether the cursor left button is currently clicking the GuiItem. */
		_Check_return_ inline bool IsLeftDown(void) const
		{
			return ldown;
		}

		/* Gets whether the cursor right button is currently clicking the GuiItem. */
		_Check_return_ inline bool IsRightDown(void) const
		{
			return rdown;
		}

		/* Gets the mesh used to render the background. */
		_Check_return_ inline const Buffer* GetBackgroundMesh(void) const
		{
			return mesh;
		}

	private:
		friend struct Menu;

		Buffer *mesh;
		TextureHandler background, focusedBackground;
		bool over, ldown, rdown, visible, enabled, focusable, focused;
		Color backColor;
		float roundingFactor;
		Rectangle bounds;
		const char *name;
		Anchors anchor;
		Vector2 offsetFromAnchorPoint;

		void CheckBounds(Vector2 size);
		void UpdateMesh(void);
		void WindowResizedHandler(WindowHandler, EventArgs);
		void MoveRelativeInternal(Anchors anchor, Vector2 base, Vector2 adder);
	};
}