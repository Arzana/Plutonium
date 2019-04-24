#include "Graphics/UI/Items/Button.h"
#include "Core/Platform/Windows/RegistryFetcher.h"

Pu::Button::Button(Application & parent, GuiBackgroundUniformBlock * backgroundDescriptor, TextUniformBlock * textDescriptor, const DescriptorSet * fontDescriptor, const Font & font)
	: Button(parent, Rectangle(0.0f, 0.0f, 0.05f, 0.04f), backgroundDescriptor, textDescriptor, fontDescriptor, font)
{}

Pu::Button::Button(Application & parent, Rectangle bounds, GuiBackgroundUniformBlock * backgroundDescriptor, TextUniformBlock * textDescriptor, const DescriptorSet * fontDescriptor, const Font & font)
	: Label(parent, bounds, backgroundDescriptor, textDescriptor, fontDescriptor, font), 
	lclickInvoked(false), rclickInvoked(false), lclickQueued(false), rclickQueued(false),
	doubleLclicked(0), doubleRclicked(0), timer(0.0f),
	LeftClicked("ButtonLeftClicked"), RightClicked("ButtonRightClicked"), DoubleClicked("ButtonDoubleClicked")
{
	threshold = GetDefaultDoubleClickThreshold();
}

Pu::Button::Button(Button && value)
	: Label(std::move(value)), lclickInvoked(value.lclickInvoked), rclickInvoked(value.rclickInvoked),
	lclickQueued(value.lclickQueued), rclickQueued(value.rclickQueued), threshold(value.threshold),
	doubleLclicked(value.doubleLclicked), doubleRclicked(value.doubleRclicked), timer(value.timer),
	LeftClicked(std::move(value.LeftClicked)), RightClicked(std::move(value.RightClicked)), DoubleClicked(std::move(DoubleClicked))
{}

void Pu::Button::Update(float dt)
{
	Label::Update(dt);

	if (IsEnabled())
	{
		/* Check if the left button is down but wait to invoke a left click until proper release to deal with double clicks. */
		if (IsLeftDown() && !lclickInvoked)
		{
			lclickInvoked = true;
			++doubleLclicked;
		}

		if (IsRightDown() && !rclickInvoked)
		{
			rclickInvoked = true;
			++doubleRclicked;
		}

		/* Update the double click timer if needed. */
		if (doubleLclicked || doubleRclicked) timer += dt;
		else timer = 0.0f;

		if (doubleLclicked > 1 || doubleRclicked > 1 || timer > threshold)
		{
			/* Handle a double click even from either the left or right click. */
			if (timer <= threshold) DoubleClicked.Post(*this);
			else
			{
				if (doubleLclicked)
				{
					/* Queue a click event for when the button is released incase of hold, otherwise just post event. */
					if (lclickInvoked) lclickQueued = true;
					else LeftClicked.Post(*this);
				}

				if (doubleRclicked)
				{
					/* Queue a click event for when the button is released incase of hold, otherwise just post event. */
					if (rclickInvoked) rclickQueued = true;
					else RightClicked.Post(*this);
				}
			}

			/* Reset counters. */
			doubleLclicked = 0;
			doubleRclicked = 0;
			timer = 0.0f;
		}

		/* Check if the left button has been released. */
		if (lclickInvoked && !IsLeftDown())
		{
			lclickInvoked = false;
			if (lclickQueued)
			{
				/* Only post the event if the user is still hovering over the button. */
				if (IsMouseOver()) LeftClicked.Post(*this);
				else lclickQueued = false;
			}
		}

		/* Same but with the right click. */
		if (rclickInvoked && !IsRightDown())
		{
			rclickInvoked = false;
			if (rclickQueued)
			{
				if (IsMouseOver()) RightClicked.Post(*this);
				else rclickQueued = false;
			}
		}
	}
}

void Pu::Button::PerformClick(MouseButtons type)
{
	switch (type)
	{
	case Pu::MouseButtons::Left:
		LeftClicked.Post(*this);
		break;
	case Pu::MouseButtons::Right:
		RightClicked.Post(*this);
		break;
	default:
		Log::Warning("Specific cursor button click is not implemented!");
		GuiItem::PerformClick();
		break;
	}
}

float Pu::Button::GetDefaultDoubleClickThreshold(void)
{
#ifdef _WIN32
	/* Cache the value so we don't query the Windows registry needlessly. */
	static float cached = 0.0f;
	if (cached != 0.0f) return cached;

	/* 
	Try to get the users Windows double click speed setting if possible; otherwise just default to 0.5 seconds.
	For some reason Windows stores this in a REG_SZ (string) so we must convert as well.
	*/
	string value;
	if (RegistryFetcher::TryReadString(L"DoubleClickSpeed", L"Control Panel\\Mouse", value))
	{
		/* Convert value from milliseconds to seconds. */
		return cached = std::stof(value) * 0.001f;
	}
#endif

	/* Return a default for other platforms or if it's missing (set to half a second). */
	return 0.5f;
}

void Pu::Button::SetDoubleClickThreshold(float value)
{
	threshold = value;
}