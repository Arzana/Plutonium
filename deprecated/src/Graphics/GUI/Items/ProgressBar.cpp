#include "Graphics\GUI\Items\ProgressBar.h"
#include "Core\Math\Interpolation.h"

Plutonium::ProgressBar::ProgressBar(Game * parent)
	: ProgressBar(parent, GetDefaultBounds())
{}

Plutonium::ProgressBar::ProgressBar(Game * parent, Rectangle bounds)
	: GuiItem(parent, bounds), style(FillStyle::LeftToRight), value(0.0f),
	bar(nullptr), barColor(GetDefaultBarColor()), INIT_BUS(ValueChanged),
	INIT_BUS(BarColorChanged), INIT_BUS(BarImageChanged), INIT_BUS(FillStyleChanged)
{
	/* Initialize bar render position. */
	OnMoved(this, ValueChangedEventArgs<Vector2>(GetPosition(), GetPosition()));
	Moved.Add(this, &ProgressBar::OnMoved);

	/* Initialize bar mesh. */
	barMesh = new Buffer(parent->GetGraphics()->GetWindow(), BindTarget::Array);
	barMesh->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, 6);
	UpdateBarMesh();
}

Plutonium::ProgressBar::~ProgressBar(void)
{
	Moved.Remove(this, &ProgressBar::OnMoved);
	delete_s(barMesh);
}

float Plutonium::ProgressBar::GetValueMapped(float min, float max) const
{
	return lerp(min, max, value);
}

void Plutonium::ProgressBar::Draw(GuiItemRenderer * renderer)
{
	GuiItem::Draw(renderer);
	if (IsVisible()) RenderProgressBar(renderer);
}

/* Warning cause has been checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::ProgressBar::SetFillStyle(FillStyle style)
{
	if (style == this->style) return;

	ValueChangedEventArgs<FillStyle> args(this->style, style);
	this->style = style;
	OnMoved(this, ValueChangedEventArgs<Vector2>(GetPosition(), GetPosition()));
	UpdateBarMesh();

	FillStyleChanged.Post(this, args);
}
#pragma warning (pop)

/* Warning cause has been checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::ProgressBar::SetValueMapped(float value, float min, float max)
{
	SetValue(ilerp(min, max, value));
}
#pragma warning (pop)

/* Warning cause has been checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::ProgressBar::SetValue(float value)
{
	if (value == this->value) return;
	LOG_THROW_IF(value < 0.0f || value > 1.0f, "Value must be between zero and one!");

	ValueChangedEventArgs<float> args(this->value, value);
	this->value = value;

	OnMoved(this, ValueChangedEventArgs<Vector2>(GetPosition(), GetPosition()));
	UpdateBarMesh();

	ValueChanged.Post(this, args);
}
#pragma warning (pop)

void Plutonium::ProgressBar::SetBarImage(TextureHandler image)
{
	if (image == bar) return;

	ValueChangedEventArgs<TextureHandler> args(bar, image);
	bar = image;
	BarImageChanged.Post(this, args);
}

void Plutonium::ProgressBar::SetBarColor(Color color)
{
	if (color == barColor) return;

	ValueChangedEventArgs<Color> args(barColor, color);
	barColor = color;
	BarColorChanged.Post(this, args);
}

void Plutonium::ProgressBar::RenderProgressBar(GuiItemRenderer * renderer)
{
	renderer->RenderBarForeground(barPos, GetBounds(), GetRoundingFactor(), barColor, bar, barMesh);
}

void Plutonium::ProgressBar::OnMoved(const GuiItem *, ValueChangedEventArgs<Vector2>)
{
	switch (style)
	{
	case FillStyle::LeftToRight:
	case FillStyle::TopToBottom:
		barPos = Vector2::Zero();
		break;
	case FillStyle::RightToLeft:
		barPos = Vector2(lerp(0.0f, GetSize().X, 1.0f - value), 0.0f);
		break;
	case FillStyle::BottomToTop:
		barPos = Vector2(0.0f, lerp(0.0f, GetSize().Y, 1.0f - value));
		break;
	default:
		LOG_WAR("Cannot set bar position for set fill style!");
		break;
	}

	barPos += GetBounds().Position;
}

void Plutonium::ProgressBar::UpdateBarMesh(void)
{
	/* Temp variables. */
	Vector2 barSize, uv1, uv2, uv3, uv4;
	const float w = GetSize().X;
	const float h = GetSize().Y;
	
	/* Gets size of the bar and set the uv coordinates. */
	switch (style)
	{
	case FillStyle::LeftToRight:
		barSize = Vector2(GetValueMapped(0.0f, w), h);
		uv1 = Vector2::Zero();
		uv2 = Vector2(value, 0.0f);
		uv3 = Vector2(value, 1.0f);
		uv4 = Vector2(0.0f, 1.0f);
		break;
	case FillStyle::RightToLeft:
		barSize = Vector2(GetValueMapped(0.0f, w), h);
		uv1 = Vector2(value, 0.0f);
		uv2 = Vector2::Zero();
		uv3 = Vector2(0.0f, 1.0f);
		uv4 = Vector2(value, 1.0f);
		break;
	case FillStyle::TopToBottom:
		barSize = Vector2(w, GetValueMapped(0.0f, h));
		uv1 = Vector2::Zero();
		uv2 = Vector2(1.0f, 0.0f);
		uv3 = Vector2(1.0f, value);
		uv4 = Vector2(0.0f, value);
		break;
	case FillStyle::BottomToTop:
		barSize = Vector2(w, GetValueMapped(0.0f, h));
		uv1 = Vector2(0.0f, value);
		uv2 = Vector2(1.0f, value);
		uv3 = Vector2(1.0f, 0.0f);
		uv4 = Vector2(0.0f, 0.0f);
		break;
	}

	/* Create and push mesh data. */
	const Vector4 data[] =
	{
		Vector4(0.0f, 0.0f, uv4.X, uv4.Y),				// bottom-left
		Vector4(0.0f, -barSize.Y, uv1.X, uv1.Y),		// top-left
		Vector4(barSize.X, -barSize.Y, uv2.X, uv2.Y),	// top-right
		Vector4(0.0f, 0.0f, uv4.X, uv4.Y),				// bottom-left
		Vector4(barSize.X, -barSize.Y, uv2.X, uv2.Y),	// top-right
		Vector4(barSize.X, 0.0f, uv3.X, uv3.Y)			// bottom-right
	};

	barMesh->SetData(data);
}