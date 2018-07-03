#pragma once
#include "Graphics\GUI\Core\CursorButtons.h"
#include "Label.h"

namespace Plutonium
{
	/* Defines a base or indipendent button object. */
	struct Button
		: public Label
	{
	public:
		/* Occurs once the button is clicked with the left cursor button. */
		EventBus<Button, CursorHandler> LeftClicked;
		/* Occurs once the button is clicked with the right cursor button. */
		EventBus<Button, CursorHandler> RightClicked;
		/* Occurs once the button is clicked twice in quick succession by either the left or right cursor button. */
		EventBus<Button, CursorHandler> DoubleClicked;
		/* Occurs when the HoverImage is set or changed. */
		EventBus<Button, ValueChangedEventArgs<TextureHandler>> HoverImageChanged;
		/* Occurs when the ClickImage is set or changed. */
		EventBus<Button, ValueChangedEventArgs<TextureHandler>> ClickImageChanged;

		/* Initializes a new instance of a button with default settings. */
		Button(_In_ Game *parent, _In_ const Font *font);
		/* Initializes a new instance of a button with a specified size. */
		Button(_In_ Game *parent, _In_ Rectangle bounds, _In_ const Font *font);
		Button(_In_ const Button &value) = delete;
		Button(_In_ Button &&value) = delete;

		_Check_return_ Button& operator =(_In_ const Button &other) = delete;
		_Check_return_ Button& operator =(_In_ Button &&other) = delete;

		/* Updates the button checking for click events. */
		virtual void Update(_In_ float dt) override;
		/* Renders the button to the screen. */
		virtual void Draw(_In_ GuiItemRenderer *renderer) override;
		/* Simulates a specific cursor click event. */
		void PerformClick(_In_ CursorButtons type = CursorButtons::Default);

		/* Gets the default value for the double click timer threshold. */
		_Check_return_ float GetDefaultDoubleClickThreshold(void) const;

		/* Gets the current hover image (nullptr is none is set). */
		_Check_return_ inline TextureHandler GetHoverImage(void) const
		{
			return hover;
		}

		/* gets the current click image (nullptr is none is set). */
		_Check_return_ inline TextureHandler GetClickImage(void) const
		{
			return click;
		}

		/* Gets the current double click timer threshold. */
		_Check_return_ inline float GetDoubleClickThreshold(void) const
		{
			return threshold;
		}

		/* Sets the background image to use when the user hovers over this button. */
		void SetHoverImage(_In_ TextureHandler image);
		/* Sets the background image to use when the user clicks on this button.  */
		void SetClickImage(_In_ TextureHandler image);
		/* Sets the threshold timer for the double click logic. */
		void SetDoubleClickThreshold(_In_ float value);

	protected:
		/* Renderes the Button to the renderer, use for internal item skipping. */
		void RenderButton(_In_ GuiItemRenderer *renderer);
		/* Gets the required size of the Button at any time, max of background or focus image. */
		_Check_return_ virtual Vector2 GetMinSize(void) const override;

	private:
		bool leftInvoked, rightInvoked, leftPostQueued, rightPostQueued;
		int32 doubleLeftClicked, doubleRightClicked, displayTex;
		float timer, threshold;
		TextureHandler hover, click;
	};
}