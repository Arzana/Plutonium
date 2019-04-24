#pragma once
#include "Label.h"

namespace Pu
{
	/* Defines a base or independent button object. */
	class Button
		: public Label
	{
	public:
		/* Occurs once the button is clicked with the left cursor button. */
		EventBus<Button> LeftClicked;
		/* Occurs once the button is clicked with the right cursor button. */
		EventBus<Button> RightClicked;
		/* Occurs once the button is clicked twice in quick succession by either the left or right cursor button. */
		EventBus<Button> DoubleClicked;

		/* Initializes a new instance of a button with default settings. */
		Button(_In_ Application &parent, _In_ GuiItemRenderer &renderer, _In_ const Font &font);
		/* Initializes a new instance of a button with a specified size. */
		Button(_In_ Application &parent, _In_ Rectangle bounds, _In_ GuiItemRenderer &renderer, _In_ const Font &font);
		Button(_In_ const Button&) = delete;
		/* Move constructor. */
		Button(_In_ Button &&value);

		_Check_return_ Button& operator =(_In_ const Button&) = delete;
		_Check_return_ Button& operator =(_In_ Button&&) = delete;

		/* Updates the button checking for click events. */
		virtual void Update(_In_ float dt) override;
		/* Simulates a specific cursor click event. */
		void PerformClick(_In_ MouseButtons type);

		/* Gets the default value for the double click timer threshold. */
		_Check_return_ static float GetDefaultDoubleClickThreshold(void);

		/* Gets the current double click timer threshold. */
		_Check_return_ inline float GetDoubleClickThreshold(void) const
		{
			return threshold;
		}

		/* Sets the threshold timer for the double click logic. */
		void SetDoubleClickThreshold(_In_ float value);

	private:
		bool lclickInvoked, rclickInvoked, lclickQueued, rclickQueued;
		int32 doubleLclicked, doubleRclicked;
		float timer, threshold;
	};
}