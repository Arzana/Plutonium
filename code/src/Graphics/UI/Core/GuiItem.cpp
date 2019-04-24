#include "Graphics/UI/Core/GuiItem.h"
#include "Graphics/Resources/DynamicBuffer.h"
#include "Graphics/VertexLayouts/Image2D.h"
#include "Graphics/UI/Rendering/GuiBackgroundUniformBlock.h"

Pu::GuiItem::GuiItem(Application & parent, GuiBackgroundUniformBlock * descriptor)
	: GuiItem(parent, Rectangle(0.0f, 0.0f, 0.05f, 0.04f), descriptor)
{}

Pu::GuiItem::GuiItem(Application & parent, Rectangle bounds, GuiBackgroundUniformBlock * descriptor)
	: Component(parent), parent(nullptr), container(nullptr), buffer(nullptr), view(nullptr),
	over(false), ldown(false), rdown(false), lclickInvoked(false), rclickInvoked(false),
	visible(true), focusable(false), focused(false), backgroundDescriptor(descriptor),
	position(bounds.Position), anchor(Anchors::None), bounds(bounds), SuppressUpdate(false),
	SuppressRender(false), BackColorChanged("GuiItemBackColorChanged"), Clicked("GuiItemClicked"),
	FocusableChanged("GuiItemFocusableChanged"), GainedFocus("GuiItemGainedFocus"),
	HoverEnter("GuiItemHoverEnter"), HoverLeave("GuiItemHoverLeave"), LostFocus("GuiItemLostFocus"),
	Moved("GuiItemMovde"), NameChanged("GuiItemNameChanged"), Resized("GuiItemResized"),
	VisibilityChanged("GuiItemVisibilityChanged")
{
	/* Initialize the descriptor to its default values. */
	descriptor->SetModel(Matrix::CreateTranslation(position));
	descriptor->SetColor(Color::Black() * 0.5f);

	/* Make sure we update the anchors if the window size changes. */
	App.GetWindow().GetNative().OnSizeChanged.Add(*this, &GuiItem::WindowResizedHandler);

	/* Create our background mesh buffer. */
	buffer = new DynamicBuffer(parent.GetDevice(), sizeof(Image2D) * 6, BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer);
	view = new BufferView(*buffer, sizeof(Image2D));
}

Pu::GuiItem::GuiItem(GuiItem && value)
	: Component(std::move(value)), parent(value.parent), container(value.container), buffer(value.buffer),
	view(value.view), over(value.over), ldown(value.ldown), rdown(value.rdown), visible(value.visible),
	lclickInvoked(value.lclickInvoked), rclickInvoked(value.rclickInvoked), focusable(value.focusable),
	focused(value.focused), backgroundDescriptor(value.backgroundDescriptor), position(value.position),
	bounds(value.bounds), name(std::move(value.name)), anchor(value.anchor), offsetFromAnchorPoint(value.offsetFromAnchorPoint),
	SuppressUpdate(value.SuppressUpdate), SuppressRender(value.SuppressRender), BackColorChanged(std::move(value.BackColorChanged)),
	Clicked(std::move(value.Clicked)), FocusableChanged(std::move(value.FocusableChanged)), GainedFocus(std::move(value.GainedFocus)),
	HoverEnter(std::move(value.HoverEnter)), HoverLeave(std::move(value.HoverLeave)), LostFocus(std::move(value.LostFocus)),
	Moved(std::move(value.Moved)), NameChanged(std::move(value.NameChanged)), Resized(std::move(value.Resized)),
	VisibilityChanged(std::move(value.VisibilityChanged))
{
	/* Make sure we update the anchors if the window size changes. */
	App.GetWindow().GetNative().OnSizeChanged.Add(*this, &GuiItem::WindowResizedHandler);

	value.buffer = nullptr;
	value.view = nullptr;
	value.backgroundDescriptor = nullptr;
}

Pu::GuiItem::~GuiItem(void)
{
	App.GetWindow().GetNative().OnSizeChanged.Remove(*this, &GuiItem::WindowResizedHandler);

	if (view) delete view;
	if (buffer) delete buffer;
	if (backgroundDescriptor) delete backgroundDescriptor;
}

void Pu::GuiItem::Initialize(void)
{
	Component::Initialize();

	/* We can only properly set the bounds here because a virtual call might be needed. */
	bounds = Rectangle(position + GetBackgroundOffset(), bounds.Size);
	CheckBounds(bounds.Size);
	UpdateMesh();
}

void Pu::GuiItem::RenderGuiItem(GuiItemRenderer & renderer) const
{
	/* Only render if the background will be visible. */
	if (GetBackColor().A > 0) renderer.EnqueueBackground(*this, false);
}

void Pu::GuiItem::MoveRelative(Anchors value, float x, float y)
{
	MoveRelativeInternal(value, Vector2(x, y), Vector2());
}

void Pu::GuiItem::PerformClick(void)
{
	Clicked.Post(*this);
}

void Pu::GuiItem::Update(float)
{
	/* Skip the update if requested. */
	if (SuppressUpdate)
	{
		SuppressUpdate = false;
		return;
	}

	/* Only update if needed. */
	if (IsEnabled())
	{
		/*
		Check for hover enter and leave events.
		We need to convert from screen space ([0, 0] to [w, h]) where the cursor lives to
		UI item space ([0, 0] to [1, 1]). We do this by first going to clip space ([-1, -1] to [1, 1])
		and then converting to UI space.
		*/
		const Vector4 cursorPos = App.GetWindow().ToLinearClipSpace(Vector4(Mouse::GetPosition(App.GetWindow().GetNative()), 0.0f, 1.0f));
		const bool newOver = bounds.Contains((cursorPos.XY + 1.0f) * 0.5f);
		if (!over && newOver) HoverEnter.Post(*this);
		else if (over && !newOver) HoverLeave.Post(*this);

		/* Check for click events. */
		if (over = newOver)
		{
			/* We only post one event per click and we also don't want two click events if the user presses left first and the right. */
			if (!(lclickInvoked || rclickInvoked))
			{
				if (ldown)
				{
					/* User clicked using the left button. */
					lclickInvoked = true;
					Clicked.Post(*this);
				}
				else if (rdown)
				{
					/* User clicked using the right button. */
					rclickInvoked = true;
					Clicked.Post(*this);
				}
			}

			/* Reset the event states. */
			if (lclickInvoked && !ldown) lclickInvoked = false;
			if (rclickInvoked && !rdown) rclickInvoked = false;
		}
	}
}

void Pu::GuiItem::Render(GuiItemRenderer & renderer) const
{
	if (visible) RenderGuiItem(renderer);
}

void Pu::GuiItem::Show(void)
{
	Enable();
	SetVisibility(true);
}

void Pu::GuiItem::Hide(void)
{
	Disable();
	SetVisibility(false);
}

void Pu::GuiItem::SetAnchors(Anchors value, float xOffset, float yOffset)
{
	/* Make sure the new anchor value differs and that it's valid. */
	if (value == anchor) return;
	if (!IsAnchorValid(value)) Log::Fatal("Invalid anchor value passed!");

	/* Update the anchor and the offset. */
	offsetFromAnchorPoint = Vector2(xOffset, yOffset);
	if ((anchor = value) != Anchors::None) MoveRelativeInternal(anchor, Vector2(), offsetFromAnchorPoint);
}

void Pu::GuiItem::SetBackColor(Color color)
{
	if (color == GetBackColor()) return;

	ValueChangedEventArgs<Color> args(GetBackColor(), color);
	backgroundDescriptor->SetColor(color);

	BackColorChanged.Post(*this, args);
}

void Pu::GuiItem::SetBounds(Rectangle value)
{
	SetPosition(value.Position);
	SetSize(value.Size);
}

void Pu::GuiItem::SetHeight(int32 height)
{
	SetSize(Vector2(bounds.GetWidth(), static_cast<float>(height)));
}

void Pu::GuiItem::SetName(const string & value)
{
	if (value == name) return;

	ValueChangedEventArgs<string> args(name, value);
	name = value;
	NameChanged.Post(*this, args);
}

void Pu::GuiItem::SetPosition(Vector2 value)
{
	if (value == position) return;

	ValueChangedEventArgs<Vector2> args(position, value);
	UpdatePosition(value);
	Moved.Post(*this, args);
}

void Pu::GuiItem::SetSize(Vector2 size)
{
	if (size == bounds.Size) return;
	CheckBounds(size);

	ValueChangedEventArgs<Vector2> args(bounds.Size, size);
	bounds.Size = size;

	UpdateMesh();
	if (anchor != Anchors::None) MoveRelativeInternal(anchor, Vector2(), offsetFromAnchorPoint);

	Resized.Post(*this, args);
}

void Pu::GuiItem::SetVisibility(bool visibility)
{
	if (visibility == visible) return;

	ValueChangedEventArgs<bool> args(visible, visibility);
	visible = visibility;
	VisibilityChanged.Post(*this, args);
}

void Pu::GuiItem::SetWidth(int32 width)
{
	SetSize(Vector2(static_cast<float>(width), bounds.GetHeight()));
}

void Pu::GuiItem::SetX(float x)
{
	SetPosition(Vector2(x, GetY()));
}

void Pu::GuiItem::SetY(float y)
{
	SetPosition(Vector2(GetX(), y));
}

void Pu::GuiItem::SetFocusable(bool value)
{
	if (value == focusable) return;

	ValueChangedEventArgs<bool> args(focusable, value);
	focusable = value;
	FocusableChanged.Post(*this, args);
}

void Pu::GuiItem::SetParent(const GuiItem & item)
{
	/* Make sure to remove the old event handlers from the GuiItem to avoid parent mismatch. */
	if (parent)
	{
		parent->Moved.Remove(*this, &GuiItem::ParentMovedHandler);
		parent->Resized.Remove(*this, &GuiItem::ParentResizedHandler);
	}

	/* Set the parent and add event handlers to make sure the position of the child is updated correctly. */
	parent = &item;
	parent->Moved.Add(*this, &GuiItem::ParentMovedHandler);
	parent->Resized.Add(*this, &GuiItem::ParentResizedHandler);

	/* Update the anchors if needed, otherwise update the position. */
	if (anchor != Anchors::None) MoveRelativeInternal(anchor, Vector2(), offsetFromAnchorPoint);
	else
	{
		UpdatePosition(position);
		Moved.Post(*this, ValueChangedEventArgs<Vector2>(position, position));
	}
}

Pu::Vector2 Pu::GuiItem::GetMinSize(void) const
{
	//TODO: add image size check here.
	return Vector2();
}

void Pu::GuiItem::ApplyFocus(bool value)
{
	if (!focusable || value == focused) return;

	if (focused = value) GainedFocus.Post(*this);
	else LostFocus.Post(*this);
}

void Pu::GuiItem::CheckBounds(Vector2 size)
{
	if (size.X <= 0.0f || size.Y <= 0.0f) Log::Fatal("Width (%d) or height (%d) of GUI item must be greater than zero!", ipart(size.X), ipart(size.Y));
}

void Pu::GuiItem::UpdateMesh(void)
{
	/*
	We need to convert the coordinates from normalized viewport space ([0,0] to [1, 1]) to clip space [-1, -1] to [1, 1].
	The coordinates are in this system to force the use of relative positions and sizes to the screen.
	This allows for a consistent UI for every display.
	*/
	const Vector2 tl = Vector2(-1.0f);
	const Vector2 br = GetSize() * 2.0f - 1.0f;

	buffer->BeginMemoryTransfer();
	Image2D *data = reinterpret_cast<Image2D*>(buffer->GetHostMemory());

	data[0] = Image2D(tl, 0.0f, 1.0f);
	data[1] = Image2D(tl.X, br.Y, 0.0f, 0.0f);
	data[2] = Image2D(br, 1.0f, 0.0f);
	data[3] = Image2D(tl, 0.0f, 1.0f);
	data[4] = Image2D(br, 1.0f, 0.0f);
	data[5] = Image2D(br.X, tl.Y, 1.0f, 1.0f);

	buffer->EndMemoryTransfer();
}

void Pu::GuiItem::UpdatePosition(Vector2 value)
{
	const Vector2 offset = GetBackgroundOffset();

	position = value;
	if (offset.X < 0.0f) position.X -= offset.X;
	if (offset.Y < 0.0f) position.Y -= offset.Y;

	bounds.Position = position + offset;
	if (parent) bounds.Position += parent->GetBoundingBox().Position;

	/* We have to scale by 2 to go from normalized viewport space offsets ([0, 0] to [1, 1]) to clip space offsets ([0, 0] to [2, 2]). */
	backgroundDescriptor->SetModel(Matrix::CreateTranslation(bounds.Position * 2.0f));
}

void Pu::GuiItem::WindowResizedHandler(const NativeWindow&, ValueChangedEventArgs<Vector2>)
{
	if (anchor != Anchors::None) MoveRelative(anchor);
}

void Pu::GuiItem::ParentMovedHandler(GuiItem & sender, ValueChangedEventArgs<Vector2>)
{
	bounds.Position = sender.GetBoundingBox().Position + position + GetBackgroundOffset();
	Moved.Post(*this, ValueChangedEventArgs<Vector2>(position, position));
}

void Pu::GuiItem::ParentResizedHandler(GuiItem &, ValueChangedEventArgs<Vector2>)
{
	if (anchor != Anchors::None) MoveRelativeInternal(anchor, Vector2(), offsetFromAnchorPoint);
}

void Pu::GuiItem::MoveRelativeInternal(Anchors value, Vector2 base, Vector2 adder)
{
	/* Get the area that the GUI item will bind itself to. Either the parent item or the viewport. */
	Rectangle area;
	if (parent) area = parent->GetBoundingBox();
	else
	{
		const Viewport vp = App.GetWindow().GetNative().GetClientBounds();
		area = Rectangle(vp.X, vp.Y, vp.Width, vp.Height);
	}

	Vector2 newPos = base;

	/* Check for horizontal anchors. */
	if (_CrtEnumCheckFlag(value, Anchors::CenterWidth)) newPos.X = area.GetWidth() / 2.0f - (GetWidth() >> 1);
	if (_CrtEnumCheckFlag(value, Anchors::Left)) newPos.X = 0.0f;
	if (_CrtEnumCheckFlag(value, Anchors::Right)) newPos.X = area.GetRight() - GetWidth();

	/* Check for vertical anchors. */
	if (_CrtEnumCheckFlag(value, Anchors::CenterHeight)) newPos.Y = area.GetHeight() / 2.0f - (GetHeight() >> 1);
	if (_CrtEnumCheckFlag(value, Anchors::Top)) newPos.Y = 0.0f;
	if (_CrtEnumCheckFlag(value, Anchors::Bottom)) newPos.Y = area.GetBottom() - GetHeight();

	/* Move the GUI item if needed. */
	newPos += adder;
	SetPosition(newPos);
}