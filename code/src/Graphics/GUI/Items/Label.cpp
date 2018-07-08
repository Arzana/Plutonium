#include "Graphics\GUI\Items\Label.h"
#include "Core\StringFunctions.h"

Plutonium::Label::Label(Game * parent, const Font * font)
	: Label(parent, GetDefaultBounds(), font)
{}

Plutonium::Label::Label(Game * parent, Rectangle bounds, const Font * font)
	: GuiItem(parent, bounds), autoSize(false), textColor(GetDefaultTextColor()),
	font(font), text(heapwstr("")), offset(GetDefaultTextOffset()), charBufferSize(GetDefaultBufferSize()),
	INIT_BUS(TextChanged), INIT_BUS(TextColorChanged), INIT_BUS(TextOffsetChanged), bindFunc()
{
	/* Initilaize text render position. */
	OnMoved(this, ValueChangedEventArgs<Vector2>(GetPosition(), GetPosition()));
	Moved.Add(this, &Label::OnMoved);
	BackgroundImageChanged.Add([&](const GuiItem*, ValueChangedEventArgs<TextureHandler> args) { HandleAutoSize(); });

	/* Initialize text mesh. */
	textMesh = new Buffer(parent->GetGraphics()->GetWindow(), BindTarget::Array);
	textMesh->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, charBufferSize * 6);
	UpdateTextMesh();
}

Plutonium::Label::~Label(void)
{
	Moved.Remove(this, &Label::OnMoved);
	free_s(text);
	delete_s(textMesh);
}

size_t Plutonium::Label::GetLineCount(void) const
{
	return cntchar(text, U'\n') + 1;
}

void Plutonium::Label::Update(float dt)
{
	GuiItem::Update(dt);

	if (IsEnabled())
	{
		/* If the bind function is set; update the labels text. */
		if (bindFunc != 0)
		{
			std::string result;
			bindFunc.HandlePost(this, result);
			SetText(result.c_str());
		}
	}
}

void Plutonium::Label::Draw(GuiItemRenderer * renderer)
{
	GuiItem::Draw(renderer);
	if (IsVisible()) RenderLabel(renderer);
}

void Plutonium::Label::SetAutoSize(bool value)
{
	if (autoSize = value) HandleAutoSize();
}

void Plutonium::Label::SetText(const char32 * text)
{
	if (eqlstr(this->text, text)) return;

	ValueChangedEventArgs<const char32*> args(this->text, text);
	this->text = heapwstr(text);
	HandleAutoSize();
	UpdateTextMesh();
	TextChanged.Post(this, args);

	free_s(const_cast<const char32*>(args.OldValue));
}

void Plutonium::Label::SetText(const char * text)
{
	char32 *str = heapwstr(text);
	SetText(str);
	free_s(str);
}

void Plutonium::Label::SetTextColor(Color color)
{
	if (textColor == color) return;

	ValueChangedEventArgs<Color> args(textColor, color);
	textColor = color;
	TextColorChanged.Post(this, args);
}

void Plutonium::Label::SetTextOffset(Vector2 offset)
{
	if (this->offset == offset) return;

	ValueChangedEventArgs<Vector2> args(this->offset, offset);
	this->offset = offset;
	textPos = GetPosition() + offset;
	HandleAutoSize();
	TextOffsetChanged.Post(this, args);
}

void Plutonium::Label::SetTextBind(Binder & binder)
{
	bindFunc = binder;
}

void Plutonium::Label::RenderLabel(GuiItemRenderer * renderer)
{
	renderer->RenderTextForeground(textPos, textColor, font, text, textMesh);
}

void Plutonium::Label::HandleAutoSize(void)
{
	if (autoSize)
	{
		/* Autosize will resize the Label to the size of the text if defined; otherwise to the size of the minimum defined size (textures), it cannot resize to a zero dimention. */
		Vector2 size = GetSize();
		Vector2 dim = strlen(text) > 0 ? dim = font->MeasureString(text) + offset * 2.0f : GetMinSize();

		if (dim != Vector2::Zero && (dim.X != size.X || dim.Y != size.Y)) SetSize(dim);
	}
}

void Plutonium::Label::OnMoved(const GuiItem *, ValueChangedEventArgs<Vector2> args)
{
	const Vector2 baseOffset = -Vector2(0.0f, GetRoundingFactor() * 0.5f);
	textPos = args.NewValue + baseOffset + offset;
}

void Plutonium::Label::UpdateTextMesh(void)
{
	/* Create CPU side vertices buffer. (6 vertices per quad of vector4 type per glyph). */
	size_t len = strlen(text), size = len * 6;
	if (len < 1) return;

	float lh = font->MeasureStringHeight(text);
	Vector4 *vertices = malloca_s(Vector4, size);

	/* Increase buffered size if needed. */
	if (len > charBufferSize)
	{
#if defined(DEBUG)
		LOG_WAR("Increasing GPU text buffer size of Label '%s' from %d to %d!", GetName(), charBufferSize, len);
#endif

		charBufferSize = len;

		free_s(textMesh);
		textMesh = new Buffer(game->GetGraphics()->GetWindow(), BindTarget::Array);
		textMesh->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, charBufferSize);
	}

	float xAdder = 0.0f;
	float yAdder = -lh;
	for (size_t i = 0, j = 0; i < len; i++)
	{
		/* Get current character. */
		const Character *ch = font->GetCharOrDefault(text[i]);

		/* Defines components of the position vertices. */
		float w = ch->Size.X;
		float h = ch->Size.Y;
		float x = xAdder + ch->Bearing.X;
		float y = yAdder - (ch->Size.Y + ch->Bearing.Y);
		Vector2 tl = ch->Bounds.Position;
		Vector2 br = ch->Bounds.Position + ch->Bounds.Size;

		/* Populate buffer. */
		vertices[j++] = Vector4(x, y + h, tl.X, tl.Y);
		vertices[j++] = Vector4(x, y, tl.X, br.Y);
		vertices[j++] = Vector4(x + w, y, br.X, br.Y);
		vertices[j++] = Vector4(x, y + h, tl.X, tl.Y);
		vertices[j++] = Vector4(x + w, y, br.X, br.Y);
		vertices[j++] = Vector4(x + w, y + h, br.X, tl.Y);

		xAdder += ch->Advance;
		if (ch->Key == U'\n') yAdder -= lh;
	}

	/* Create GPU side buffer. */
	textMesh->SetData(vertices, size);
	freea_s(vertices);
}