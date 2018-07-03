#include "Graphics\GUI\Core\GuiItem.h"
#include "Core\StringFunctions.h"

Plutonium::GuiItem::GuiItem(Game * parent)
	: GuiItem(parent, GetDefaultBounds())
{}

Plutonium::GuiItem::GuiItem(Game * parent, Rectangle bounds)
	: game(parent), background(nullptr), focusedBackground(nullptr), backColor(GetDefaultBackColor()),
	over(false), ldown(false), rdown(false), visible(false), enabled(false), 
	sizable(false), focusable(false), focused(false), movable(false),
	roundingFactor(10.0f), bounds(bounds), name(heapstr("<Unnamed GuiItem>")),
	suppressRefresh(false), suppressUpdate(false), suppressRender(false),
	INIT_BUS(BackColorChanged), INIT_BUS(BackgroundImageChanged), INIT_BUS(Clicked),
	INIT_BUS(Finalized), INIT_BUS(Hover), INIT_BUS(HoverEnter), INIT_BUS(FocusableChanged),
	INIT_BUS(HoverLeave), INIT_BUS(Moved), INIT_BUS(NameChanged), INIT_BUS(Resized),
	INIT_BUS(SizableChanged), INIT_BUS(MovableChanged), INIT_BUS(StateChanged), INIT_BUS(VisibilityChanged),
	INIT_BUS(GainedFocus), INIT_BUS(LostFocus), INIT_BUS(FocusedImageChanged)
{
	/* Check for invalid bounds and show the GuiItem. */
	CheckBounds(bounds.Size);

	/* Initialize mesh. */
	mesh = new Buffer(parent->GetGraphics()->GetWindow(), BindTarget::Array);
	mesh->SetData<Vector4>(BufferUsage::StaticDraw, nullptr, 6);
	UpdateMesh();

	Show();
}

Plutonium::GuiItem::~GuiItem(void)
{
	/* Call finalize before releaseing assets. */
	Finalized.Post(this, EventArgs());

	free_s(name);
	delete_s(mesh);
}

void Plutonium::GuiItem::PerformClick(void)
{
	Clicked.Post(this, game->GetCursor());
}

void Plutonium::GuiItem::Update(float dt)
{
	/* Skip update call if requested. */
	if (suppressUpdate)
	{
		suppressUpdate = false;
		return;
	}

	/* Only update the GuiItem is needed. */
	if (enabled)
	{
		CursorHandler cursor = game->GetCursor();

		/* Check for hover enter and leave events. */
		bool newOver = bounds.Contains(GetRotatedCursor(cursor));
		if (!over && newOver) HoverEnter.Post(this, cursor);
		else if (over && !newOver) HoverLeave.Post(this, cursor);

		/* Check for click events. */
		if (over = newOver)
		{
			bool down = cursor->LeftButton || cursor->RightButton;

			/* Check for hover event. */
			if (!down && !(ldown || rdown)) Hover.Post(this, cursor);
			/* Check for left click event. */
			else if (cursor->LeftButton && !(ldown || rdown))
			{
				Clicked.Post(this, cursor);
				ldown = true;
			}
			/* Check for right click event. */
			else if (cursor->RightButton && !(ldown || rdown))
			{
				Clicked.Post(this, cursor);
				rdown = true;
			}
		}

		/* Reset clicked state. */
		if (ldown && !cursor->LeftButton) ldown = false;
		if (rdown && !cursor->RightButton) rdown = false;
	}
}

void Plutonium::GuiItem::Draw(GuiItemRenderer * renderer)
{
	if (visible) RenderGuiItem(renderer);
}

void Plutonium::GuiItem::MoveRelative(Anchors anchor, float x, float y)
{
	/* Checks whether the anchor is valid. */
	if (_CrtIsAnchorWorkable(anchor))
	{
		const Rectangle vp = game->GetGraphics()->GetWindow()->GetClientBounds();

		/* Check for horizontal anchors. */
		if (_CrtEnumCheckFlag(anchor, Anchors::CenterWidth)) x = vp.GetWidth() / 2.0f - (GetWidth() >> 1);
		if (_CrtEnumCheckFlag(anchor, Anchors::Left)) x = vp.GetLeft();
		if (_CrtEnumCheckFlag(anchor, Anchors::Right)) x = vp.GetRight() - GetWidth();

		/* Check for vertical anchors. */
		if (_CrtEnumCheckFlag(anchor, Anchors::CenterHeight)) y = vp.GetHeight() / 2.0f - (GetHeight() >> 1);
		if (_CrtEnumCheckFlag(anchor, Anchors::Top)) y = vp.GetTop();
		if (_CrtEnumCheckFlag(anchor, Anchors::Bottom)) y = vp.GetBottom() - GetHeight();

		/* Move GuiItem if needed. */
		if (x != GetX() || y != GetY()) SetPosition(Vector2(x, y));
	}
	else LOG_THROW_IF(!_CrtIsAnchorValid(anchor), "The anchor value is inalid!");
}

void Plutonium::GuiItem::Show(void)
{
	SetState(true);
	SetVisibility(true);
}

void Plutonium::GuiItem::Hide(void)
{
	SetState(false);
	SetVisibility(false);
}

void Plutonium::GuiItem::SetBackColor(Color color)
{
	if (color == backColor) return;

	ValueChangedEventArgs<Color> args(backColor, color);
	backColor = color;
	BackColorChanged.Post(this, args);
}

void Plutonium::GuiItem::SetBackgroundImage(TextureHandler image)
{
	if (image == background) return;

	ValueChangedEventArgs<TextureHandler> args(background, image);
	background = image;
	BackgroundImageChanged.Post(this, args);
}

void Plutonium::GuiItem::SetFocusedBackgroundImage(TextureHandler image)
{
	if (image == focusedBackground) return;

	ValueChangedEventArgs<TextureHandler> args(focusedBackground, image);
	focusedBackground = image;
	FocusedImageChanged.Post(this, args);
}

void Plutonium::GuiItem::SetBounds(Rectangle bounds)
{
	SetPosition(bounds.Position);
	SetSize(bounds.Size);
}

void Plutonium::GuiItem::SetState(bool enabled)
{
	if (enabled == this->enabled) return;

	ValueChangedEventArgs<bool> args(this->enabled, enabled);
	this->enabled = enabled;
	StateChanged.Post(this, args);
}

void Plutonium::GuiItem::SetHeight(int32 height)
{
	SetSize(Vector2(bounds.GetWidth(), static_cast<float>(height)));
}

void Plutonium::GuiItem::SetName(const char * name)
{
	if (eqlstr(this->name, name)) return;

	ValueChangedEventArgs<const char*> args(this->name, name);
	this->name = heapstr(name);
	NameChanged.Post(this, args);

	free_s(const_cast<const char*>(args.OldValue));
}

void Plutonium::GuiItem::SetPosition(Vector2 position)
{
	if (position == bounds.Position) return;

	ValueChangedEventArgs<Vector2> args(bounds.Position, position);
	bounds.Position = position;
	Moved.Post(this, args);
}

void Plutonium::GuiItem::SetSize(Vector2 size)
{
	if (size == bounds.Size) return;
	CheckBounds(size);

	ValueChangedEventArgs<Vector2> args(bounds.Size, size);
	bounds.Size = size;
	UpdateMesh();
	Resized.Post(this, args);
}

void Plutonium::GuiItem::SetVisibility(bool visibility)
{
	if (visibility == visible) return;

	ValueChangedEventArgs<bool> args(visible, visibility);
	visible = visibility;
	VisibilityChanged.Post(this, args);
}

void Plutonium::GuiItem::SetWidth(int32 width)
{
	SetSize(Vector2(static_cast<float>(width), bounds.GetHeight()));
}

void Plutonium::GuiItem::SetX(float x)
{
	SetPosition(Vector2(x, GetY()));
}

void Plutonium::GuiItem::SetY(float y)
{
	SetPosition(Vector2(GetX(), y));
}

void Plutonium::GuiItem::SetSizable(bool value)
{
	if (value == sizable) return;

	ValueChangedEventArgs<bool> args(sizable, value);
	sizable = value;
	SizableChanged.Post(this, args);
}

void Plutonium::GuiItem::SetMovable(bool value)
{
	if (value == movable) return;

	ValueChangedEventArgs<bool> args(movable, value);
	movable = value;
	MovableChanged.Post(this, args);
}

void Plutonium::GuiItem::SetFocusable(bool value)
{
	if (value == focusable) return;

	ValueChangedEventArgs<bool> args(focusable, value);
	focusable = value;
	FocusableChanged.Post(this, args);
}

void Plutonium::GuiItem::RenderGuiItem(GuiItemRenderer * renderer)
{
	renderer->RenderGuiItem(bounds, roundingFactor, 0.0f, backColor, focused ? focusedBackground : background, sizable, mesh);
}

Plutonium::Vector2 Plutonium::GuiItem::GetRotatedCursor(CursorHandler cursor)
{
	Vector3 transformedPos = Matrix::CreateRotationZ(-0.0f) * Vector3(cursor->X - GetX(), cursor->Y - GetY(), 0.0f);
	return GetPosition() + Vector2(transformedPos.X, transformedPos.Y);
}

Plutonium::Vector2 Plutonium::GuiItem::GetMinSize(void) const
{
	return max(background ? background->GetSize() : Vector2::Zero, focusedBackground ? focusedBackground->GetSize() : Vector2::Zero);
}

void Plutonium::GuiItem::CheckBounds(Vector2 size)
{
	LOG_THROW_IF(size.X <= 0.0f || size.Y <= 0.0f, "Width(%d) or height(%d) of GuiItem must be greater then zero!", ipart(size.X), ipart(size.Y));
}

void Plutonium::GuiItem::UpdateMesh(void)
{
	const Vector4 data[] =
	{
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),					// bottom-left
		Vector4(0.0f, -bounds.Size.Y, 0.0f, 0.0f),			// top-left
		Vector4(bounds.Size.X, -bounds.Size.Y, 1.0f, 0.0f),	// top-right
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),					// bottom-left
		Vector4(bounds.Size.X, -bounds.Size.Y, 1.0f, 0.0f),	// top-right
		Vector4(bounds.Size.X, 0.0f, 1.0f, 1.0f)			// bottom-right
	};

	mesh->SetData(data);
}

void Plutonium::GuiItem::ApplyFocus(bool focused)
{
	if (!focusable || focused == this->focused) return;

	if (this->focused = focused) GainedFocus.Post(this, EventArgs());
	else LostFocus.Post(this, EventArgs());
}