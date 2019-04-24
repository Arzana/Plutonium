#pragma once
#include "Graphics/Text/TextUniformBlock.h"
#include "Graphics/UI/Core/GuiItem.h"
#include "Graphics/Text/Font.h"

namespace Pu
{
	class TextBuffer;

	/* Defines a base or independent item for displaying text. */
	class Label
		: public GuiItem
	{
	public:
		/* Occurs when the text property is changed. */
		EventBus<Label, ValueChangedEventArgs<ustring>> TextChanged;
		/* Occurs when the text color property is changed. */
		EventBus<Label, ValueChangedEventArgs<Color>> TextColorChanged;
		/* Occurs when the text offset property is changed. */
		EventBus<Label, ValueChangedEventArgs<Vector2>> TextOffsetChanged;

		/* Initializes a new instance of a label with default parameters. */
		Label(_In_ Application &parent, _In_ GuiItemRenderer &renderer, _In_ const Font &font);
		/* Initializes a new instance of a label with default parameters. */
		Label(_In_ Application &parent, _In_ Rectangle bounds, _In_ GuiItemRenderer &renderer, _In_ const Font &font);
		Label(_In_ const Label&) = delete;
		/* Move constructor. */
		Label(_In_ Label &&value);
		/* Releases the resources allocated by the label. */
		virtual ~Label(void);

		_Check_return_ Label& operator =(_In_ const Label &other) = delete;
		_Check_return_ Label& operator =(_In_ Label &&other) = delete;

		/* Gets the amount of lines within this label (at least 1). */
		_Check_return_ size_t GetLineCount(void) const;
		/* Renders the label to the display. */
		virtual void Render(_In_ GuiItemRenderer &renderer) const override;

		/* Gets whether the label should be automatically resized to fit the text. */
		_Check_return_ inline bool GetAutoSize(void) const
		{
			return autoSize;
		}

		/* Gets the curent displayed text. */
		_Check_return_ inline const ustring& GetText(void) const
		{
			return text;
		}

		/* Gets the current text color. */
		_Check_return_ inline Color GetTextColor(void) const
		{
			return textDescriptor->GetColor();
		}

		/* Gets the offset from the background render position to the text render position. */
		_Check_return_ inline Vector2 GetTextOffset(void) const
		{
			return offset;
		}

		/* Gets the font used to render the Label. */
		_Check_return_ inline const Font& GetFont(void) const
		{
			return *font;
		}

		/* Sets whether the label should automatically resize to fit the text. */
		void SetAutoSize(_In_ bool value);
		/* Sets the text to be displayed. */
		void SetText(_In_ const ustring &value);
		/* Sets the color with which to render the text. */
		void SetTextColor(_In_ Color value);
		/* Sets the offset from the background position to the text position. */
		void SetTextOffset(_In_ Vector2 value);

	protected:
		/* Handles the autosize functionality. */
		virtual void HandleAutoSize(void);
		/* Renders the Label to the renderer, used for internal item skipping. */
		void RenderLabel(_In_ GuiItemRenderer &renderer) const;
		/* Sets the visible text to a specified value, used to override the visible text of the label. */
		void SetVisualString(_In_ const ustring &string);

		/* Gets the string that will be rendered by the label. */
		_Check_return_ inline const ustring& GetVisualString(void) const
		{
			return visibleText;
		}

	private:
		friend class GuiItemRenderer;

		bool autoSize;
		ustring text, visibleText;
		const Font *font;
		Vector2 offset;

		TextBuffer *textBuffer;
		TextUniformBlock *textDescriptor;
		const DescriptorSet *fontDescriptor;

		void OnMoved(GuiItem&, ValueChangedEventArgs<Vector2>);
		void UpdateTextMesh(void);
	};
}