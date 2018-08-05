#include "Graphics\GUI\Items\Slider.h"
#include "Core\Math\VInterpolation.h"

Plutonium::Slider::Slider(Game * parent)
	: Slider(parent, GetDefaultBounds())
{}

Plutonium::Slider::Slider(Game * parent, Rectangle bounds)
	: ProgressBar(parent, bounds), holderBarEnabled(true), allowRandomClicks(true),
	dragInvoked(false), holderBarColor(GetDefaultHolderBarColor()),
	holderBar(nullptr), holderBarBounds(Vector2::Zero(), GetDefaultHolderBarSize()),
	INIT_BUS(DragStart), INIT_BUS(DragEnd), INIT_BUS(HolderBarColorChanged),
	INIT_BUS(HolderBarImageChanged), INIT_BUS(HolderBarResized)
{
	/* Initialize holder bar render position. */
	UpdateHolderBarPos();
	Moved.Add([&](const GuiItem*, ValueChangedEventArgs<Vector2>) { UpdateHolderBarPos(); });
	ValueChanged.Add([&](const ProgressBar*, ValueChangedEventArgs<float>) { UpdateHolderBarPos(); });
	FillStyleChanged.Add([&](const ProgressBar*, ValueChangedEventArgs<FillStyle>) { UpdateHolderBarPos(); });
	Clicked.Add(this, &Slider::OnRandomClick);

	/* Initialize holder bar mesh. */
	holderBarMesh = new Buffer(parent->GetGraphics()->GetWindow(), BindTarget::Array);
	holderBarMesh->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, 6);
	UpdateHolderBarMesh();
}

Plutonium::Slider::~Slider(void)
{
	Clicked.Remove(this, &Slider::OnRandomClick);
	delete_s(holderBarMesh);
}

void Plutonium::Slider::Update(float dt)
{
	ProgressBar::Update(dt);

	/* Only check for events if the slider is enabled. */
	if (IsEnabled())
	{
		/* If the holder bar is enabled; check for drag events. */
		if (holderBarEnabled)
		{
			CursorHandler cursor = game->GetCursor();

			/* Check if a drag event has started. */
			if (cursor->LeftButton)
			{
				
				if (!dragInvoked && holderBarBounds.Contains(cursor->GetPosition()))
				{
					dragInvoked = true;
					DragStart.Post(this, cursor);
				}
			}

			/* If the user is currently dragging the slider holder bar. */
			if (dragInvoked)
			{
				/* Check if the drag event has ended. */
				if (!cursor->LeftButton)
				{
					dragInvoked = false;
					DragEnd.Post(this, cursor);
				}
				else
				{
					/* 
					Move the cursor into slider space.
					Then project it onto the size of the slider.
					Map the projection from slider space into amount space.
					Finally clamp the value between zero and one.
					*/
					Vector2 b = GetSize();
					float bl = b.Length();
					float proj = dot(cursor->GetPosition() - GetPosition(), b) / bl;

					float desired = 0.0f;
					switch (GetFillStyle())
					{
					case (FillStyle::LeftToRight):
					case (FillStyle::TopToBottom):
						desired = ilerp(0.0f, bl, proj);
						break;
					case (FillStyle::RightToLeft):
					case (FillStyle::BottomToTop):
						desired = ilerp(bl, 0.0f, proj);
						break;
					}

					SetValue(clamp(desired, 0.0f, 1.0f));
				}
			}
		}
	}
}

void Plutonium::Slider::Draw(GuiItemRenderer * renderer)
{
	/* Only draw the hold bar if desired. */
	ProgressBar::Draw(renderer);
	if (IsVisible() && holderBarEnabled) RenderSlider(renderer);
}

void Plutonium::Slider::SetHolderBarState(bool enabled)
{
	holderBarEnabled = enabled;
}

void Plutonium::Slider::SetRandomClicks(bool allow)
{
	allowRandomClicks = allow;
}

void Plutonium::Slider::SetHolderBarColor(Color color)
{
	if (color == holderBarColor) return;

	ValueChangedEventArgs<Color> args(holderBarColor, color);
	holderBarColor = color;
	HolderBarColorChanged.Post(this, args);
}

void Plutonium::Slider::SetHolderBarImage(TextureHandler image)
{
	if (image == holderBar) return;

	ValueChangedEventArgs<TextureHandler> args(holderBar, image);
	holderBar = image;
	HolderBarImageChanged.Post(this, args);
}

void Plutonium::Slider::SetHolderBarSize(Vector2 value)
{
	if (value == holderBarBounds.Size) return;

	ValueChangedEventArgs<Vector2> args(holderBarBounds.Size, value);
	holderBarBounds.Size = value;
	UpdateHolderBarPos();
	UpdateHolderBarMesh();

	HolderBarResized.Post(this, args);
}

void Plutonium::Slider::RenderSlider(GuiItemRenderer * renderer)
{
	renderer->RenderBackground(holderBarBounds, 0.0f, holderBarColor, holderBar, holderBarMesh, true);
}

void Plutonium::Slider::OnRandomClick(const GuiItem *, CursorHandler cursor)
{
	if (allowRandomClicks && !dragInvoked)
	{
		dragInvoked = true;
		DragStart.Post(this, cursor);
	}
}

void Plutonium::Slider::UpdateHolderBarPos(void)
{
	Vector2 offset;
	Vector2 size = GetBounds().Size;

	/*
	We want the holder bar to never leave the defined bounds of the background (progress bar), 
	but we also want the user to have accurate control over the value specified.
	For this we will draw the holder bar not at the actual position but always at a slight offset.
	Because of this we will have a slight change in accuracy when the slider bar is in the center compared to the sides.
	*/
	switch (GetFillStyle())
	{
	case (FillStyle::LeftToRight):
		offset = Vector2(lerp(0.0f, size.X - holderBarBounds.GetWidth(), GetValue()), -holderBarBounds.GetHeight() * 0.5f + size.Y * 0.5f);
		break;
	case (FillStyle::RightToLeft):
		offset = Vector2(lerp(size.X - holderBarBounds.GetWidth(), 0.0f, GetValue()), -holderBarBounds.GetHeight() * 0.5f + size.Y * 0.5f);
		break;
	case (FillStyle::TopToBottom):
		offset = Vector2(-holderBarBounds.GetWidth() * 0.5f + size.X * 0.5f, lerp(0.0f, size.Y - holderBarBounds.GetHeight(), GetValue()));
		break;
	case (FillStyle::BottomToTop):
		offset = Vector2(-holderBarBounds.GetWidth() * 0.5f + size.X * 0.5f, lerp(size.Y - holderBarBounds.GetHeight(), 0.0f, GetValue()));
		break;
	default:
		LOG_WAR("Cannot set holder bar position for set fill style!");
		break;
	}

	holderBarBounds.Position = GetBounds().Position + offset;
}

void Plutonium::Slider::UpdateHolderBarMesh(void)
{
	const float w = holderBarBounds.GetWidth();
	const float h = holderBarBounds.GetHeight();

	const Vector4 data[] =
	{
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),	// bottom-left
		Vector4(0.0f, -h, 0.0f, 0.0f),		// top-left
		Vector4(w, -h, 1.0f, 0.0f),			// top-right
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),	// bottom-left
		Vector4(w, -h, 1.0f, 0.0f),			// top-right
		Vector4(w, 0.0f, 1.0f, 1.0f)		// bottom-right
	};

	holderBarMesh->SetData(data);
}