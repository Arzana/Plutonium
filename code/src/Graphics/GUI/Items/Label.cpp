#include "Graphics\GUI\Items\Label.h"
#include "Core\StringFunctions.h"

Plutonium::Label::Label(Game * parent, const Font * font)
	: Label(parent, GetDefaultBounds(), font)
{}

Plutonium::Label::Label(Game * parent, Rectangle bounds, const Font * font)
	: GuiItem(parent, bounds), autoSize(false), textColor(GetDefaultTextColor()),
	font(font), text(heapwstr("")), visibleText(heapwstr("")), offset(GetDefaultTextOffset()), charBufferSize(GetDefaultBufferSize()),
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
	free_s(visibleText);
	delete_s(textMesh);
}

size_t Plutonium::Label::GetLineCount(void) const
{
	return cntchar(visibleText, U'\n') + 1;
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
	free_s(visibleText);
	visibleText = heapwstr(text);
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
	const Vector2 baseOffset = -Vector2(0.0f, GetRoundingFactor() * 0.5f);
	textPos = GetBounds().Position + baseOffset + offset;
	HandleAutoSize();
	TextOffsetChanged.Post(this, args);
}

void Plutonium::Label::SetTextBind(Binder & binder)
{
	bindFunc = binder;
}

void Plutonium::Label::RenderLabel(GuiItemRenderer * renderer)
{
	if (strlen(visibleText) > 0) renderer->RenderTextForeground(textPos, textColor, font, textMesh);
}

void Plutonium::Label::SetVisualString(const char32 * string)
{
	free_s(visibleText);
	visibleText = heapwstr(string);
	HandleAutoSize();
	UpdateTextMesh();
}

void Plutonium::Label::HandleAutoSize(void)
{
	if (autoSize)
	{
		/* Autosize will resize the Label to the size of the text if defined; otherwise to the size of the minimum defined size (textures), it cannot resize to a zero dimention. */
		Vector2 size = GetSize();
		Vector2 dim = strlen(visibleText) > 0 ? dim = font->MeasureString(visibleText) + offset * 2.0f : GetMinSize();

		if (dim != Vector2::Zero() && (dim.X != size.X || dim.Y != size.Y)) SetSize(dim);
	}
}

void Plutonium::Label::OnMoved(const GuiItem *, ValueChangedEventArgs<Vector2> args)
{
	const Vector2 baseOffset = -Vector2(0.0f, GetRoundingFactor() * 0.5f);
	textPos = GetBounds().Position + baseOffset + offset;
}

void Plutonium::Label::UpdateTextMesh(void)
{
	/* Create CPU side vertices buffer. (6 vertices per quad of vector4 type per glyph). */
	size_t len = strlen(visibleText), size = len * 6;
	if (len < 1) return;

	float lh = static_cast<float>(font->GetLineSpace());
	Vector4 *vertices = malloca_s(Vector4, size);

	/* Increase buffered size if needed. */
	if (len > charBufferSize)
	{
		size_t newBufferSize = max(len, charBufferSize << 1);

#if defined(DEBUG)
		LOG_WAR("Increasing GPU text buffer size of Label '%s' from %d to %d!", GetName(), charBufferSize, newBufferSize);
#endif

		charBufferSize = newBufferSize;

		delete_s(textMesh);
		textMesh = new Buffer(game->GetGraphics()->GetWindow(), BindTarget::Array);
		textMesh->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, charBufferSize * 6);
	}

	float xAdder = 0.0f;
	float yAdder = -lh;
	size_t j = 0;
	for (size_t i = 0; i < len; i++)
	{
		char32 c = visibleText[i];

		/* Hanlde newlines. */
		if (c == U'\n')
		{
			xAdder = 0.0f;
			yAdder -= lh;
			continue;
		}
		else if (c == U'\r') continue;

		/* Get current character. */
		const Character *ch = font->GetCharOrDefault(c);

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
	}

	/* Create GPU side buffer. */
	textMesh->SetData(vertices, j);
	freea_s(vertices);
}