#pragma once
#include "Graphics\GUI\Containers\Container.h"
#include "Graphics\GUI\Items\Button.h"

namespace Plutonium
{
	/* Defines a basic window control. */
	struct GUIWindow
		: public GuiItem, protected Container
	{
		/* Occurs when the window close button is pressed. */
		EventBus<GUIWindow, EventArgs> Closed;
		/* Occurs when the user starts holding the header bar. */
		EventBus<GUIWindow, CursorHandler> DragStart;
		/* Occurs when the user stops holding the header bar. */
		EventBus<GUIWindow, CursorHandler> DragEnd;
		/* Occurs when the color of the header bar is changed. */
		EventBus<GUIWindow, ValueChangedEventArgs<Color>> HeaderBarColorChanged;

		/* Initializes a new instance of a window. */
		GUIWindow(_In_ Game *parent, _In_ const Font *font);
		/* Initializes a new instance of a window with a specified position and size. */
		GUIWindow(_In_ Game *parent, _In_ Rectangle bounds, _In_ const Font *font);
		GUIWindow(_In_ const GUIWindow &value) = delete;
		GUIWindow(_In_ GUIWindow &&value) = delete;
		/* Releases the resources allocated by the window. */
		~GUIWindow(void);

		/* Updates the Window and it's underlying components. */
		virtual void Update(_In_ float dt) override;
		/* Renders the Window and it's underlying components to the screen. */
		virtual void Draw(_In_ GuiItemRenderer *renderer) override;

		/* Gets the default value for the Window bounds. */
		_Check_return_ static inline Rectangle GetDefaultBounds(void)
		{
			return Rectangle(0.0f, 0.0f, 100.0f, 100.0f);
		}

		/* Gets the default color for the Window header bar. */
		_Check_return_ static inline Color GetDefaultHeaderColor(void)
		{
			return Color::WhiteSmoke();
		}

		/* Gets the displayed title of the Window. */
		_Check_return_ inline const char32* GetTitle(void) const
		{
			return lblName->GetText();
		}

		/* Gets whether the user is able to move the window by dragging the header bar. */
		_Check_return_ inline bool IsDragable(void) const
		{
			return userCanDrag;
		}

		/* Gets whether the use is able to minimize the window. */
		_Check_return_ inline bool IsMinimizeable(void) const
		{
			return userCanMinimize;
		}

		/* Gets whether the window will resize if the controlls go out of bounds. */
		_Check_return_ inline bool IsAutoSize(void) const
		{
			return autoSize;
		}

		/* gets the current color of the header bar. */
		_Check_return_ inline Color GetHeaderColor(void) const
		{
			return hdrClr;
		}

		/* Gets the bounding box of the GuiItem. */
		_Check_return_ virtual Rectangle GetBoundingBox(void) const;
		/* Sets the window title to the specified value. */
		void SetTitle(_In_ const char32 *title);
		/* Sets whether the user is allowed to drag the window. */
		void SetAllowDrag(_In_ bool allowed);
		/* Sets whether the user is allowed to minimize the window. */
		void SetAllowMinimize(_In_ bool allowed);
		/* Sets the color of the header bar. */
		void SetHeaderColor(_In_ Color value);
		/* Sets the color of the header text. */
		void SetHeaderTextColor(_In_ Color value);
		/* Adds a GuiItem to this window. */
		void AddItem(_In_ GuiItem *item);

	protected:
		/* Renders the window to the renderer, use for internal item skipping. */
		void RenderWindow(_In_ GuiItemRenderer *renderer);

		/* Gets the offset that shuold be used for the background bounds. */
		_Check_return_ virtual inline Vector2 GetBackgroundOffset(void) const override
		{
			return Vector2(0.0f, -hdrSpltH);
		}

	private:
		Button * btnExit, *btnMin;
		Label *lblName;
		float hdrSpltH;
		bool userCanDrag, userCanMinimize, autoSize;
		bool dragInvoked, minimized;
		Color hdrClr;

		void HandleAutoSize(void);
		void UpdateHdrHeight(void);
		void OnChildResized(const GuiItem*, ValueChangedEventArgs<Vector2>);
		void OnExitButtonPressed(const Button*, CursorHandler);
		void OnMinButtonPressed(const Button*, CursorHandler);
	};
}