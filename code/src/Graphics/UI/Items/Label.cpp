#include "Graphics/UI/Items/Label.h"
#include "Graphics/Text/TextBuffer.h"
#include "Graphics/Text/TextRenderer.h"

Pu::Label::Label(Application & parent, GuiItemRenderer & renderer, const Font & font)
	: Label(parent, Rectangle(0.0f, 0.0f, 0.05f, 0.04f), renderer, font)
{}

Pu::Label::Label(Application & parent, Rectangle bounds, GuiItemRenderer & renderer, const Font & font)
	: GuiItem(parent, bounds, renderer), autoSize(false), font(&font), offset(0.005f),
	TextChanged("LabelTextChanged"), TextColorChanged("LabelTextColorChanges"), TextOffsetChanged("LabelTextOffsetChanged")
{
	/* Initialize the font and text descriptor. */
	fontDescriptor = renderer.GetTextRenderer().CreatFont(font.GetAtlas());
	textDescriptor = renderer.GetTextRenderer().CreateText();
	textDescriptor->SetColor(Color::White());

	/* Initialize the text position. */
	Moved.Add(*this, &Label::OnMoved);
	OnMoved(*this, ValueChangedEventArgs<Vector2>(Vector2(), Vector2()));	// Args aren't used.

	/* Allocate the text mesh. */
	textBuffer = new TextBuffer(App.GetDevice(), 64);
	UpdateTextMesh();
}

Pu::Label::Label(Label && value)
	: GuiItem(std::move(value)), autoSize(value.autoSize), text(std::move(value.text)),
	visibleText(std::move(value.visibleText)), font(value.font), offset(value.offset),
	textBuffer(value.textBuffer), textDescriptor(value.textDescriptor),
	fontDescriptor(value.fontDescriptor), TextChanged(std::move(value.TextChanged)),
	TextColorChanged(std::move(value.TextColorChanged)), TextOffsetChanged(std::move(TextOffsetChanged))
{
	value.textBuffer = nullptr;
	value.textDescriptor = nullptr;
	value.fontDescriptor = nullptr;
}

Pu::Label::~Label(void)
{
	Moved.Remove(*this, &Label::OnMoved);

	if (textBuffer) delete textBuffer;
	if (textDescriptor) delete textDescriptor;
	if (fontDescriptor) delete fontDescriptor;
}

size_t Pu::Label::GetLineCount(void) const
{
	return visibleText.count(U'\n') + 1;
}

void Pu::Label::Render(GuiItemRenderer & renderer) const
{
	GuiItem::Render(renderer);
	if (IsVisible()) RenderLabel(renderer);
}

void Pu::Label::SetAutoSize(bool value)
{
	autoSize = value;
	HandleAutoSize();
}

void Pu::Label::SetText(const ustring & value)
{
	if (value == text) return;

	ValueChangedEventArgs<ustring> args(text, value);
	text = value;

	SetVisualString(text);
	TextChanged.Post(*this, args);
}

void Pu::Label::SetTextColor(Color value)
{
	if (value == GetTextColor()) return;

	ValueChangedEventArgs<Color> args(GetTextColor(), value);
	textDescriptor->SetColor(value);
	TextColorChanged.Post(*this, args);
}

void Pu::Label::SetTextOffset(Vector2 value)
{
	if (value == offset) return;

	ValueChangedEventArgs<Vector2> args(offset, value);
	offset = value;

	OnMoved(*this, args);	// Args aren't used so just pass this structure.
	HandleAutoSize();

	TextOffsetChanged.Post(*this, args);
}

void Pu::Label::HandleAutoSize(void)
{
	if (autoSize)
	{
		/* Autosize will resize the Label to the size of the text if defined; otherwise to the size of the minimum defined size (textures), it cannot resize to a zero dimention. */
		const Vector2 size = GetSize();
		Vector2 dim;

		/* If the text isn't set we just use the minimum size. */
		if (visibleText.length() > 0)
		{
			/* 
			We need to convert the size to clip space because that's where we're rendering to. 
			So convert to go from clip space ([-1, -1] to [1, 1]) to size space ([0, 0] to [1, 1]).
			We also add the offset twice, once for each side.
			*/
			dim = (App.GetWindow().ToLinearClipSpace(Vector4(font->MeasureString(visibleText), 0.0f, 1.0f)).XY + 1.0f) * 0.5f;
			dim += offset * 2.0f;
		}
		else dim = GetMinSize();

		/* Only update the dimensions if they are not equal and if the dimension is greater than zero. */
		if (dim != Vector2() && (dim.X != size.X || dim.Y != size.Y)) SetSize(dim);
	}
}

void Pu::Label::RenderLabel(GuiItemRenderer & renderer) const
{
	/* Only render if there is text and it's visible. */
	if (visibleText.length() > 0 && GetTextColor().A > 0) renderer.EnqueueText(*this);
}

void Pu::Label::SetVisualString(const ustring & string)
{
	visibleText = string;
	HandleAutoSize();
	UpdateTextMesh();
}

void Pu::Label::OnMoved(GuiItem &, ValueChangedEventArgs<Vector2>)
{
	textDescriptor->SetModel(Matrix::CreateTranslation(GetBounds().Position + offset));
}

void Pu::Label::UpdateTextMesh(void)
{
	textBuffer->SetText(visibleText, *font, App.GetWindow());
}