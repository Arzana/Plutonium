#pragma once
#include "Anchors.h"
#include "Application.h"
#include "Components/Component.h"
#include "Graphics/UI/Rendering/GuiItemRenderer.h"
#include "Graphics/UI/Rendering/GuiBackgroundUniformBlock.h"

namespace Pu
{
	class GuiItemContainer;
	class DynamicBuffer;

	/* Defines thed base object for all GUI items. */
	class GuiItem
		: public Component
	{
	public:
		/* Occurs when the BackColor value is changed. */
		EventBus<GuiItem, ValueChangedEventArgs<Color>> BackColorChanged;
		/* Occurs when the GuiItem is clicked with any button. */
		EventBus<GuiItem> Clicked;
		/* Occurs when the Focusable indicator is changed. */
		EventBus<GuiItem, ValueChangedEventArgs<bool>> FocusableChanged;
		/* Occurs when the GuiItem gains focus. */
		EventBus<GuiItem> GainedFocus;
		/* Occurs when the cursor pointer enters the GuiItem's bounds. */
		EventBus<GuiItem> HoverEnter;
		/* Occurs when the cursor pointer leaves the GuiItem's bounds. */
		EventBus<GuiItem> HoverLeave;
		/* Occurs when the GuiItem loses focus. */
		EventBus<GuiItem> LostFocus;
		/* Occurs when the position of the GuiItem is changed. */
		EventBus<GuiItem, ValueChangedEventArgs<Vector2>> Moved;
		/* Occurs when the name of the GuiItem is set or changed. */
		EventBus<GuiItem, ValueChangedEventArgs<string>> NameChanged;
		/* Occurs when the GuiItem is resized. */
		EventBus<GuiItem, ValueChangedEventArgs<Vector2>> Resized;
		/* Occurs when the GuiItem is shown or hiden. */
		EventBus<GuiItem, ValueChangedEventArgs<bool>> VisibilityChanged;

		/* Initializes a new instance of a GUI item with default parameters. */
		GuiItem(_In_ Application &parent, _In_ GuiItemRenderer &renderer);
		/* Initializes a new instance of a base GUI item with a specified position and size. */
		GuiItem(_In_ Application &parent, _In_ Rectangle bounds, _In_ GuiItemRenderer &renderer);
		GuiItem(_In_ const GuiItem&) = delete;
		/* Move constructor. */
		GuiItem(_In_ GuiItem &&value);
		/* Releases the resources allocated by the GuiItem. */
		virtual ~GuiItem(void);

		_Check_return_ GuiItem& operator =(_In_ const GuiItem&) = delete;
		_Check_return_ GuiItem& operator =(_In_ GuiItem&&) = delete;

		/*
		Moves the GuiItem to a specified relative position.
		The anchor will have prefrence over the specified positional components.
		*/
		void MoveRelative(_In_ Anchors value, _In_opt_ float x = 0.0f, _In_opt_ float y = 0.0f);
		/* Simulated a cursor click event. */
		void PerformClick(void);
		/* Updates the UI item, checking if any event are occuring. */
		virtual void Update(_In_ float);
		/* Renders the UI item to the display. */
		virtual void Render(_In_ GuiItemRenderer &renderer) const;
		/* Enables the GuiItem and makes it visible. */
		void Show(void);
		/* Disables the GuiItem and makes it hiden. */
		void Hide(void);
		/* Sets the anchor to the specified value, making sure the GuiItem always stays at the desired position if the parent or GuiItem resizes. */
		virtual void SetAnchors(_In_ Anchors value, _In_opt_ float xOffset = 0.0f, _In_opt_ float yOffset = 0.0f);
		/* Sets the color of the background to a new solid color, or (when a background image is set) changes the color filter of the background image. */
		virtual void SetBackColor(_In_ Color color);
		/* Sets the bounds of the GuiItem, possibly moving it and resizing it. */
		virtual void SetBounds(_In_ Rectangle value);
		/* Sets the height of the GuiItem, resizing it. */
		virtual void SetHeight(_In_ int32 height);
		/* Sets the name indentifier of the GuiItem. */
		virtual void SetName(_In_ const string &name);
		/* Sets the position of the GuiItem. */
		virtual void SetPosition(_In_ Vector2 value);
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
		/* Sets the parent of this GuiItem. */
		virtual void SetParent(const GuiItem &item);

		/* Gets the current value of the anchor. */
		_Check_return_ inline Anchors GetAnchor(void) const
		{
			return anchor;
		}

		/* Gets the current value of the background color. */
		_Check_return_ inline Color GetBackColor(void) const
		{
			return backgroundDescriptor->GetColor();
		}

		/* Gets the bounds of the GuiItem. */
		_Check_return_ inline Rectangle GetBounds(void) const
		{
			return bounds;
		}

		/* Gets the bounding box of the GuiItem, used for anchoring. */
		_Check_return_ virtual inline Rectangle GetBoundingBox(void) const
		{
			return bounds;
		}

		/* Gets the maximum bounds of the GuiItem, this includes the background bounds and the bounding box. */
		_Check_return_ inline Rectangle GetMaxBounds(void) const
		{
			return GetBoundingBox().Merge(bounds);
		}

		/* Gets the current height of the bounds as an integer value. */
		_Check_return_ inline int32 GetHeight(void) const
		{
			return static_cast<int32>(GetMaxBounds().GetHeight());
		}

		/* Gets the assigned name of the GuiItem. */
		_Check_return_ inline const string& GetName(void) const
		{
			return name;
		}

		/* Gets the current position of the GuiItem. */
		_Check_return_ inline Vector2 GetPosition(void) const
		{
			return GetMaxBounds().LowerBound;
		}

		/* Gets the current size of the GuiItem. */
		_Check_return_ inline Vector2 GetSize(void) const
		{
			return GetMaxBounds().GetSize();
		}

		/* Gets whether the GuiItem is currently visible. */
		_Check_return_ inline bool IsVisible(void) const
		{
			return visible;
		}

		/* Gets the current widht of the bounds as an integer value. */
		_Check_return_ inline int32 GetWidth(void) const
		{
			return static_cast<int32>(GetMaxBounds().GetWidth());
		}

		/* Gets the current horizontal position of the GuiItem. */
		_Check_return_ inline float GetX(void) const
		{
			return GetPosition().X;
		}

		/* Gets the current vertical position of the GuiItem. */
		_Check_return_ inline float GetY(void) const
		{
			return GetPosition().Y;
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

		/* Gets the current offset from the defined anchor point. */
		_Check_return_ inline Vector2 GetOffsetFromAnchor(void) const
		{
			return offsetFromAnchorPoint;
		}

		/* Gets the current parent of this GuiItem, can be nullptr! */
		_Check_return_ inline const GuiItem* GetParent(void) const
		{
			return parent;
		}

	protected:
		/* Suppresses the next update call to this GuiItem, resetting it afterwards. */
		bool SuppressUpdate;
		/* Suppresses the next render call to this GuiItem, resetting it afterwards. */
		bool SuppressRender;

		/* Initialized the GUI item. */
		virtual void Initialize(void) override;
		/* Renders the GUI item to the renderer, used for internal item skipping. */
		void RenderGuiItem(_In_ GuiItemRenderer &renderer) const;
		/* Gets the required size of the GuiItem at any time, max of background or focus image. */
		_Check_return_ virtual Vector2 GetMinSize(void) const;
		/* This function can be called to give the GuiItem focus or have it lose focus. */
		void ApplyFocus(bool value);

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

		/* Gets the offset that shuold be used for the background bounds. */
		_Check_return_ virtual inline Vector2 GetBackgroundOffset(void) const
		{
			return Vector2();
		}

	private:
		friend class GuiItemContainer;
		friend class GuiItemRenderer;

		const GuiItem *parent;
		GuiItemContainer *container;
		DynamicBuffer *buffer;
		BufferView *view;
		GuiBackgroundUniformBlock *backgroundDescriptor;

		bool over, ldown, rdown, lclickInvoked, rclickInvoked;
		bool visible, focusable, focused;
		Vector2 position;
		Rectangle bounds;
		string name;
		Anchors anchor;
		Vector2 offsetFromAnchorPoint;

		void CheckBounds(Vector2 size);
		void UpdateMesh(void);
		void UpdatePosition(Vector2 value);
		void WindowResizedHandler(const NativeWindow&, ValueChangedEventArgs<Vector2>);
		void ParentMovedHandler(GuiItem &sender, ValueChangedEventArgs<Vector2>);
		void ParentResizedHandler(GuiItem&, ValueChangedEventArgs<Vector2>);
		void MoveRelativeInternal(Anchors value, Vector2 base, Vector2 adder);
	};
}