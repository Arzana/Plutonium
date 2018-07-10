#pragma once
#include "Label.h"
#include "Graphics\GUI\Core\InputFlags.h"
#include "Graphics\GUI\Core\GlyphRejectionArgs.h"

namespace Plutonium
{
	/* Defines a user interactable label. */
	struct TextBox
		: public Label
	{
	public:
		/* Occurs when the user presses the specified confirm key. */
		EventBus<TextBox, EventArgs> Confirmed;
		/* Occurs when the user attempts to add a glyph that is not allowed. */
		EventBus<TextBox, GlyphRejectionArgs> GlyphRejected;

		/* Initializes a new instance of a label with default settings. */
		TextBox(_In_ Game *parent, _In_ const Font *font);
		/* Initializes a new instance of a label with a specified size. */
		TextBox(_In_ Game *parent, _In_ Rectangle bounds, _In_ const Font *font);
		TextBox(_In_ const TextBox &value) = delete;
		TextBox(_In_ TextBox &&value) = delete;
		/* Releases the resource allocated by the label. */
		~TextBox(void);

		_Check_return_ TextBox& operator =(_In_ const TextBox &other) = delete;
		_Check_return_ TextBox& operator =(_In_ TextBox &&other) = delete;

		/* Updates the textbox updating its events and focus status. */
		virtual void Update(_In_ float dt);
		/* Gets the default value for the time (in seconds) that a key needs to be hold to count as a repeating key. */
		_Check_return_ static float GetDefaultKeyHoldThreshold(void);

		/* Gets the default value for the minimum size of the textbox. */
		_Check_return_ static inline Vector2 GetDefaultMinimumSize(void)
		{
			return Vector2(100.0f, 25.0f);
		}

		/* Gets the default value for the maximum size of the textbox. */
		_Check_return_ static inline Vector2 GetDefaultMaximumSize(void)
		{
			return Vector2(maxv<float>());
		}

		/* Gets the current minimum size of the text box. */
		_Check_return_ inline Vector2 GetMinimumSize(void) const
		{
			return minSize;
		}

		/* Gets the current maximum size of the text box. */
		_Check_return_ inline Vector2 GetMaximumSize(void) const
		{
			return maxSize;
		}

		/* Gets at what rate (in seconds) the text box will flicker if focused. */
		_Check_return_ inline float GetFlickerInterval(void) const
		{
			return flickerInterval;
		}

		/* Gets the current input modifiers for the text box. */
		_Check_return_ inline InputFlags GetInputType(void) const
		{
			return flags;
		}

		/* Gets the maximum allowed length for the user to input (0 indicate no max lenght). */
		_Check_return_ inline uint32 GetMaxTextLength(void) const
		{
			return maxStringLength;
		}

		/* Gets whether the user is allowed to start a new line in this text box. */
		_Check_return_ inline bool IsMultiLine(void) const 
		{
			return multiLine;
		}

		/* Gets the replacement character for this textbox (0 means this textbox is not a password field).  */
		_Check_return_ inline char32 GetPasswordChar(void) const
		{
			return passRepl;
		}

		/* Gets the key used to allow the user to confirm the textbox. */
		_Check_return_ inline Keys GetConfirmationKey(void) const
		{
			return confirm;
		}

		/* Gets the amount of time (in seconds) for a key to be hold before it counts as a repeating key. */
		_Check_return_ inline float GetKeyHoldThreshold(void) const
		{
			return holdThreshold;
		}

		/* Sets the minimum size of the textbox. */
		void SetMinimumSize(_In_ Vector2 size);
		/* Sets the maximum size of the textbox. */
		void SetMaximumSize(_In_ Vector2 size);
		/* Sets the flicker interval (in seconds), zero means always show, negative means never show. */
		void SetFlickerInterval(_In_ float seconds);
		/* Sets the flags indicating which characters are allowed to be added by the user. */
		void SetInputFlags(_In_ InputFlags flags);
		/* Sets the maximum allowed length of string in this text box. */
		void SetMaximumLength(_In_ int32 length);
		/* Sets whether the user is allowed to add new lines to the string. */
		void SetMultiLine(_In_ bool allow);
		/* Sets the replacement character used to hide the origional string. */
		void SetPassword(_In_ char32 replacement);
		/* Sets the key used to trigger a confirmation event. */
		void SetConfirmKey(_In_ Keys key);
		/* Sets the threshold (in seconds) for a key to be considered repeating. */
		void SetKeyHoldThreshold(_In_ float seconds);

	protected:
		/* Handles the autosize functionality. */
		virtual void HandleAutoSize(void) override;

	private:
		Vector2 minSize, maxSize;
		float flickerInterval, holdThreshold, flickerTimer, bsHoldTimer;
		InputFlags flags;
		uint32 maxStringLength;
		bool multiLine, showLine;
		char32 passRepl;
		Keys confirm;

		void OnGlyphInput(WindowHandler, uint32 key);
		void UpdateVisibleString(void);
	};
}