#include "Graphics\GUI\Containers\GUIWindow.h"

Plutonium::GUIWindow::GUIWindow(Game * parent, const Font * font)
	: GUIWindow(parent, GetDefaultBounds(), font)
{}

Plutonium::GUIWindow::GUIWindow(Game * parent, Rectangle bounds, const Font * font)
	: GuiItem(parent, bounds), Container(), userCanDrag(true), hdrClr(GetDefaultHeaderColor()),
	dragInvoked(false), hdrSpltH(0.0f), userCanMinimize(true), minimized(false), autoSize(true), hideOnClose(false),
	INIT_BUS(Closed), INIT_BUS(DragStart), INIT_BUS(DragEnd), INIT_BUS(HeaderBarColorChanged)
{
	ASSERT_IF(!font, "Font cannot be null!");

	float yOffset = static_cast<float>(-font->GetLineSpace());

	/* Initializes the exit button. */
	btnExit = new Button(parent, font);
	btnExit->DoubleClicked.Add(this, &GUIWindow::OnExitButtonPressed);
	btnExit->LeftClicked.Add(this, &GUIWindow::OnExitButtonPressed);
	btnExit->SetBackColor(Color::Transparent());
	btnExit->SetTextOffset(Vector2(5.0f, 0.0f));
	btnExit->SetName("WindowButtonExit");
	btnExit->SetTextColor(Color::Red());
	btnExit->SetAutoSize(true);
	btnExit->SetParent(this);
	btnExit->SetText(U"X");

	/* Initialize the minimize button. */
	btnMin = new Button(parent, font);
	btnMin->DoubleClicked.Add(this, &GUIWindow::OnMinButtonPressed);
	btnMin->LeftClicked.Add(this, &GUIWindow::OnMinButtonPressed);
	btnMin->SetBackColor(Color::Transparent());
	btnMin->SetTextOffset(Vector2(5.0f, 0.0f));
	btnMin->SetName("WindowButtonMinimize");
	btnMin->SetTextColor(Color::White());
	btnMin->SetAutoSize(true);
	btnMin->SetParent(this);
	btnMin->SetText(U"-");

	/* Initialize the window title. */
	lblName = new Label(parent, Rectangle(btnExit->GetBounds().Size), font);
	lblName->SetBackColor(Color::Transparent());
	lblName->SetName("WindowLabelTitle");
	lblName->SetAutoSize(true);
	lblName->SetParent(this);

	/* Make sure the headers height is initialized. */
	UpdateHdrHeight();

	/* Only set the header controlls to their anchor once the header size is set to ensure the correct position of the header items. */
	btnExit->SetAnchors(Anchors::TopRight, 0.0f, yOffset);
	btnMin->SetAnchors(Anchors::TopRight, -btnExit->GetSize().X, yOffset);
	lblName->SetAnchors(Anchors::TopLeft, 0.0f, yOffset);
}

Plutonium::GUIWindow::~GUIWindow(void)
{
	delete_s(btnExit);
	delete_s(btnMin);
	delete_s(lblName);
}

void Plutonium::GUIWindow::Update(float dt)
{
	GuiItem::Update(dt);

	if (IsEnabled())
	{
		if (!minimized) Container::Update(dt);
		CursorHandler cursor = game->GetCursor();

		/* Make sure to only update when the user is not dragging to make sure we don't get random window close or minimize calls. */
		if (!dragInvoked)
		{
			/* Update the header manually to make sure it's to disabled during minimize. */
			btnExit->Update(dt);
			btnMin->Update(dt);
			lblName->Update(dt);
		}

		/* Check if a drag event has started. */
		if (cursor->LeftButton && userCanDrag)
		{
			if (!dragInvoked && Rectangle(GetBounds().Position, Vector2(GetBounds().GetWidth(), hdrSpltH)).Contains(cursor->GetPosition()))
			{
				dragInvoked = true;
				DragStart.Post(this, cursor);
			}
		}

		/* If the user is currently dragging the header bar. */
		if (dragInvoked)
		{
			/* Check if the drag event has ended. */
			if (!cursor->LeftButton || !userCanDrag)
			{
				dragInvoked = false;
				DragEnd.Post(this, cursor);
			}
			else SetPosition(GetBounds().Position + cursor->GetDelta());
		}
	}
}

void Plutonium::GUIWindow::Draw(GuiItemRenderer * renderer)
{
	if (IsVisible())
	{
		RenderWindow(renderer);
		if (!minimized) Container::Render(renderer);
	}
}

Plutonium::Rectangle Plutonium::GUIWindow::GetBoundingBox(void) const
{
	Rectangle bb = GetBounds();
	Vector2 offset = Vector2(0.0f, hdrSpltH);
	return Rectangle(bb.Position + offset, bb.Size - offset);
}

void Plutonium::GUIWindow::SetTitle(const char32 * title)
{
	lblName->SetText(title);
	UpdateHdrHeight();
}

void Plutonium::GUIWindow::SetAllowDrag(bool allowed)
{
	userCanDrag = allowed;
}

void Plutonium::GUIWindow::SetAllowMinimize(bool allowed)
{
	userCanMinimize = allowed;
	btnMin->SetState(allowed);
	btnMin->SetVisibility(allowed);
	if (minimized) minimized = allowed;
}

/* Warning cause is checked and code is working as intended and assignment and and check are intended. */
#pragma warning(push)
#pragma warning(disable:4458)
#pragma warning(disable:4706)
void Plutonium::GUIWindow::SetAutoSize(bool enabled)
{
	if (autoSize = enabled) HandleAutoSize();
}
#pragma warning(pop)

void Plutonium::GUIWindow::SetCloseResponse(bool hide)
{
	hideOnClose = hide;
}

void Plutonium::GUIWindow::SetHeaderColor(Color value)
{
	if (value == hdrClr) return;

	ValueChangedEventArgs<Color> args(hdrClr, value);
	hdrClr = value;
	HeaderBarColorChanged.Post(this, args);
}

void Plutonium::GUIWindow::SetHeaderTextColor(Color value)
{
	lblName->SetTextColor(value);
}

void Plutonium::GUIWindow::AddItem(GuiItem * item)
{
	item->SetParent(this);
	item->Resized.Add(this, &GUIWindow::OnChildResized);
	Container::AddItem(item);

	HandleAutoSize();
}

void Plutonium::GUIWindow::RenderWindow(GuiItemRenderer * renderer)
{
	if (minimized)
	{
		renderer->RenderBackground(Rectangle(GetBounds().Position, Vector2(GetBounds().GetWidth(), hdrSpltH)), GetRoundingFactor(), hdrClr, GetBackgroundImage(), GetBackgroundMesh(), false);
	}
	else
	{
		renderer->RenderBackground(GetBounds(), GetRoundingFactor(), GetBackColor(), hdrClr, GetBackgroundImage(), GetBackgroundMesh(), hdrSpltH);
	}

	/* Draw header manually to make sure it's not invisible during minimize. */
	btnExit->Draw(renderer);
	btnMin->Draw(renderer);
	lblName->Draw(renderer);
}

void Plutonium::GUIWindow::HandleAutoSize(void)
{
	/* Only apply auto size if needed. */
	if (autoSize)
	{
		/* Get a rectangle that contains all childs. */
		Rectangle bb;
		for (size_t i = 0; i < GetControlCount(); i++) bb = Rectangle::Merge(bb, GetControllAt(i)->GetMaxBounds());

		/* Get the current size of the window and transform the dimentions of the rectangle into a single size value. */
		Vector2 curSize = GetSize();
		Vector2 dim = (bb.Size - bb.Position) + Vector2(0.0f, hdrSpltH);

		/* Only change the size if the dimentions will be more than zero. */
		if (dim != Vector2::Zero())
		{
			/* Make sure we only resize to make the window larger not smaller to avoid stuttering. */
			if (dim.X > curSize.X && dim.Y > curSize.Y) SetSize(dim);
			else if (dim.X > curSize.X) SetSize(Vector2(dim.X, curSize.Y));
			else if (dim.Y > curSize.Y) SetSize(Vector2(curSize.X, dim.Y));
		}
	}
}

void Plutonium::GUIWindow::UpdateHdrHeight(void)
{
	hdrSpltH = max(lblName->GetSize().Y, max(btnExit->GetSize().Y, btnMin->GetSize().Y));
}

void Plutonium::GUIWindow::OnChildResized(const GuiItem *, ValueChangedEventArgs<Vector2>)
{
	HandleAutoSize();
}

void Plutonium::GUIWindow::OnExitButtonPressed(const Button *, CursorHandler)
{
	Closed.Post(this, EventArgs());
}

void Plutonium::GUIWindow::OnMinButtonPressed(const Button *, CursorHandler)
{
	minimized = !minimized;
}