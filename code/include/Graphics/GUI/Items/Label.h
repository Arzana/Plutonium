#pragma once
#include "Graphics\GUI\Core\GuiItem.h"
#include "Graphics\Text\Font.h"

namespace Plutonium
{
	/* Defines an base or independent item for displaying text. */
	struct Label
		: public GuiItem
	{
	public:
		/* Occurs when the text property is changed. */
		EventBus<Label, ValueChangedEventArgs<const char32*>> TextChanged;
		/* Occurs when the text color property is changed. */
		EventBus<Label, ValueChangedEventArgs<Color>> TextColorChanged;
		/* Occurs when the text offset property is changed. */
		EventBus<Label, ValueChangedEventArgs<Vector2>> TextOffsetChanged;

		/* Initializes a new instance of a label with default settings. */
		Label(_In_ Game *parent, _In_ const Font *font);
		/* Initializes a new instance of a label with a specified size. */
		Label(_In_ Game *parent, _In_ Rectangle bounds, _In_ const Font *font);
		Label(_In_ const Label &value) = delete;
		Label(_In_ Label &&value) = delete;
		/* Releases the resource allocated by the label. */
		~Label(void);

		_Check_return_ Label& operator =(_In_ const Label &other) = delete;
		_Check_return_ Label& operator =(_In_ Label &&other) = delete;

		/* Gets the amount of lines within this label (at least 1). */
		_Check_return_ size_t GetLineCount(void) const;
		/* Renders the Label to the screen. */
		virtual void Draw(_In_ GuiItemRenderer *renderer);

		/* Gets whether the label should be automatically resized to fit the text. */
		_Check_return_ bool GetAutoSize(void) const
		{
			return autoSize;
		}

		/* Gets the initial size of the text buffer. */
		_Check_return_ size_t GetDefaultBufferSize(void) const
		{
			return 64;
		}

		/* Gets the default value for the text color. */
		_Check_return_ Color GetDefaultTextColor(void) const
		{
			return Color::Black;
		}

		/* Gets the default value for the text offset. */
		_Check_return_ Vector2 GetDefaultTextOffset(void) const
		{
			return Vector2(10.0f, 0.0f);
		}

		/* Gets the curent displayed text. */
		_Check_return_ const char32* GetText(void) const
		{
			return text;
		}

		/* Gets the current text color. */
		_Check_return_ Color GetTextColor(void) const
		{
			return textColor;
		}

		/* Gets the offset from the background render position to the text render position. */
		_Check_return_ Vector2 GetTextOffset(void) const
		{
			return offset;
		}

		/* Gets the font used to render the Label. */
		_Check_return_ const Font* GetFont(void) const
		{
			return font;
		}

		/* Sets whether the label should automatically resize to fit the text. */
		void SetAutoSize(_In_ bool value);
		/* Sets the text to be displayed. */
		void SetText(_In_ const char32 *text);
		/* Sets the text to be displayed. */
		void SetText(_In_ const char *text);
		/* Sets the color with which to render the text. */
		void SetTextColor(_In_ Color color);
		/* Sets the offset from the background position to the text position. */
		void SetTextOffset(_In_ Vector2 offset);

	protected:
		/* Handles the autosize functionality. */
		void HandleAutoSize(void);

	private:
		bool autoSize;
		const char32 *text;
		Color textColor;
		const Font *font;
		Vector2 offset;
		Vector2 textPos;
		Buffer *textMesh;
		size_t charBufferSize;

		void OnMoved(const GuiItem*, ValueChangedEventArgs<Vector2> args);
		void UpdateTextMesh(void);
	};
}