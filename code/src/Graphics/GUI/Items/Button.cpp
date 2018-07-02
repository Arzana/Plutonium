#include "Graphics\GUI\Items\Button.h"

Plutonium::Button::Button(Game * parent, const Font * font)
	: Button(parent, GetDefaultBounds(), font)
{}

Plutonium::Button::Button(Game * parent, Rectangle bounds, const Font * font)
	: Label(parent, bounds, font), leftInvoked(false), rightInvoked(false),
	leftPostQueued(false), rightPostQueued(false),
	doubleLeftClicked(0), doubleRightClicked(0), timer(0.0f), displayTex(0),
	threshold(GetDefaultDoubleClickThreshold()), hover(nullptr), click(nullptr),
	INIT_BUS(LeftClicked), INIT_BUS(RightClicked), INIT_BUS(DoubleClicked),
	INIT_BUS(HoverImageChanged), INIT_BUS(ClickImageChanged)
{}

void Plutonium::Button::Update(float dt)
{
	/* Update underlying label. */
	Label::Update(dt);

	if (IsEnabled())
	{
		/* Set the display texture to default or hover depending on cursor state. */
		if (!IsMouseOver()) displayTex = 0;
		else if (IsMouseOver() && !IsLeftDown() && !IsRightDown()) displayTex = 1;

		/* Check if the left button is down, set the display texture and increase the double click counter. */
		if (IsLeftDown() && !leftInvoked)
		{
			leftInvoked = true;
			++doubleLeftClicked;
			displayTex = 2;
		}

		/* Check if the right button is down, set the display texture and increase the double click counter. */
		if (IsRightDown() && !rightInvoked)
		{
			rightInvoked = true;
			++doubleRightClicked;
			displayTex = 2;
		}

		/* Only update the timer if we're waiting for a button press. */
		if (doubleLeftClicked || doubleRightClicked) timer += dt;
		else timer = 0.0f;

		/* Check if any button is pressed twice or the timer has expired. */
		if (doubleLeftClicked > 1 || doubleRightClicked > 1 || timer > threshold)
		{
			/* Check if double clicks happened before the timer expired. */
			if (timer <= threshold) DoubleClicked.Post(this, game->GetCursor());
			else
			{
				/* Make sure the correct button event is triggered once the button is released. */
				if (doubleLeftClicked) leftPostQueued = true;
				if (doubleRightClicked) rightPostQueued = true;
			}

			/* Reset counters. */
			doubleLeftClicked = 0;
			doubleRightClicked = 0;
			timer = 0.0f;
		}

		/* Check if the left button has been released. */
		if (leftInvoked && !IsLeftDown())
		{
			leftInvoked = false;
			if (leftPostQueued)
			{
				/* Post left click event if the user is still hovering over the button. */
				if (IsMouseOver()) LeftClicked.Post(this, game->GetCursor());
				leftPostQueued = false;
			}
		}

		/* Check if the right button has been released. */
		if (rightInvoked && !IsRightDown())
		{
			rightInvoked = false;
			if (rightPostQueued)
			{
				/* Post right click event if the use is still hoverig over the button. */
				if (IsMouseOver()) RightClicked.Post(this, game->GetCursor());
				rightPostQueued = false;
			}
		}
	}
}

void Plutonium::Button::Draw(GuiItemRenderer * renderer)
{
	/* We can't use the label draw method because the button need to override the gui item background image. */
	if (IsVisible())
	{
		/* Render background with specific texture. */
		TextureHandler tex = displayTex == 0 ? GetBackgroundImage() : (displayTex == 1 ? hover : click);
		renderer->RenderGuiItem(GetBounds(), GetRoundingFactor(), 0.0f, GetBackColor(), tex, IsSizable(), GetBackgroundMesh());

		/* Render foreground. */
		renderer->RenderTextForeground(GetTextRenderPosition(), 0.0f, GetTextColor(), GetFont(), GetText(), GetTextMesh());
	}
}

void Plutonium::Button::PerformClick(CursorButtons type)
{
	switch (type)
	{
	case Plutonium::CursorButtons::Default:
		GuiItem::PerformClick();
		break;
	case Plutonium::CursorButtons::Left:
		LeftClicked.Post(this, game->GetCursor());
		break;
	case Plutonium::CursorButtons::Right:
		RightClicked.Post(this, game->GetCursor());
		break;
	case Plutonium::CursorButtons::Double:
		DoubleClicked.Post(this, game->GetCursor());
		break;
	default:
		LOG_WAR("Specific cursor button click is not implemented!");
		break;
	}
}

void Plutonium::Button::SetHoverImage(TextureHandler image)
{
	if (image == hover) return;

	ValueChangedEventArgs<TextureHandler> args(hover, image);
	hover = image;
	HoverImageChanged.Post(this, args);
}

void Plutonium::Button::SetClickImage(TextureHandler image)
{
	if (image == click) return;

	ValueChangedEventArgs<TextureHandler> args(click, image);
	click = image;
	ClickImageChanged.Post(this, args);
}

void Plutonium::Button::SetDoubleClickThreshold(float value)
{
	threshold = value;
}