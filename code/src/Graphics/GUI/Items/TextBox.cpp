#pragma warning (disable:4703)

#include "Graphics\GUI\Items\TextBox.h"
#include "Core\StringFunctions.h"
#include "Core\Platform\Windows\RegistryFetcher.h"

Plutonium::TextBox::TextBox(Game * parent, const Font * font)
	: TextBox(parent, GetDefaultBounds(), font)
{}

Plutonium::TextBox::TextBox(Game * parent, Rectangle bounds, const Font * font)
	: Label(parent, bounds, font), minSize(GetDefaultMinimumSize()),
	holdThreshold(GetDefaultKeyHoldThreshold()),
	maxSize(GetDefaultMaximumSize()), flickerInterval(1.0f), flickerTimer(0.0f),
	flags(InputFlags::All), maxStringLength(0), multiLine(false), showLine(false),
	passRepl(U'\0'), confirm(Keys::Enter), INIT_BUS(Confirmed), INIT_BUS(GlyphRejected)
{
	/* Make sure the textbox is focusable. */
	SetFocusable(true);

	/* Bind unicode glyph getter. */
	game->GetKeyboard()->CharInput.Add(this, &TextBox::OnGlyphInput);

	/* Handle when focus is gained and lost by the text box controll.  */
	Clicked.Add([&](const GuiItem*, CursorHandler) { ApplyFocus(true); });
	Confirmed.Add([&](const TextBox*, EventArgs)  { ApplyFocus(false); });
	LostFocus.Add([&](const GuiItem*, EventArgs) 
	{
		showLine = false;
		UpdateVisibleString();
	});
}

Plutonium::TextBox::~TextBox(void)
{
	game->GetKeyboard()->CharInput.Remove(this, &TextBox::OnGlyphInput);
}

void Plutonium::TextBox::Update(float dt)
{
	Label::Update(dt);

	/* Update the focus indicator if the text box is enabled and focused, if not; set the line to not show. */
	if (IsEnabled() && IsFocused())
	{
		if (flickerInterval == 0.0f) showLine = true;
		else if (flickerInterval < 0.0f) showLine = false;
		else
		{
			/* Make sure we handle lag spikes correctly by removing the interval from the timer in a while loop. */
			bool shouldUpdate = false;
			flickerTimer += dt;
			while (flickerTimer >= flickerInterval)
			{
				flickerTimer -= flickerInterval;
				showLine = !showLine;
				shouldUpdate = true;
			}

			/* Only actually update the visible string once to save performance. */
			if (shouldUpdate) UpdateVisibleString();
		}

		KeyHandler kb = game->GetKeyboard();

		/* Check if the confirm key is pressed. */
		if (kb->IsKeyDown(confirm)) Confirmed.Post(this, EventArgs());

		/* Check for backspace events. */
		if (kb->IsKeyDown(Keys::Backspace))
		{
			/* If the key is first pressed or is hold for the required amount of time, apply backspace. */
			if (bsHoldTimer == 0.0f || bsHoldTimer > holdThreshold)
			{
				const char32 *curText = GetText();
				const size_t len = strlen(curText);

				/* Only apply backspace if we actually have text to remove. */
				if (len > 0)
				{
					char32 *buffer = malloca_s(char32, len);
					substr(curText, 0, len - 1, buffer);
					SetText(buffer);
					freea_s(buffer);
					UpdateVisibleString();
				}
			}

			bsHoldTimer += dt;
		}
		else bsHoldTimer = 0.0f;
	}
	else
	{
		flickerTimer = 0.0f;
		showLine = false;
	}
}

float Plutonium::TextBox::GetDefaultKeyHoldThreshold(void)
{
	constexpr float DEFAULT = 1.0f;

#if defined (_WIN32)
	/* Cache the value so we don;t query the Windows registy needlessly. */
	static float cached = 0.0f;
	if (cached != 0.0f) return cached;

	/* Try to get the users Windows keyboard hold delay setting if possible; otherwise just default to 1.0 seconds. */
	const char *value = nullptr;
	if (RegistryFetcher::TryReadString("KeyboardDelay", "Control Panel\\Keyboard", &value))
	{
		/* Convert value from string to float. */
		float seconds = strtof(value, nullptr);
		free_s(value);
		return cached = seconds;
	}
#endif

	return DEFAULT;
}

void Plutonium::TextBox::SetMinimumSize(Vector2 size)
{
	if (size == minSize) return;
	ASSERT_IF(size.X < 0.0f || size.Y < 0.0f, "Minimum size must be greater than zero!");

	minSize = size;
	HandleAutoSize();
}

void Plutonium::TextBox::SetMaximumSize(Vector2 size)
{
	if (size == maxSize) return;

	maxSize = size != Vector2::Zero() ? size : Vector2(maxv<float>());
	HandleAutoSize();
}

void Plutonium::TextBox::SetFlickerInterval(float seconds)
{
	flickerInterval = seconds;
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::TextBox::SetInputFlags(InputFlags flags)
{
	if (flags == this->flags) return;
	LOG_THROW_IF(strlen(GetText()) > 0, "Cannot change input flags after the text box has received input!");

	this->flags = flags;
}
#pragma warning (pop)

void Plutonium::TextBox::SetMaximumLength(uint32 length)
{
	if (length == maxStringLength) return;
	LOG_THROW_IF(strlen(GetText()) > 0, "Cannot change maximum string length after the text box has received input!");

	this->maxStringLength = length;
}

void Plutonium::TextBox::SetMultiLine(bool allow)
{
	multiLine = allow;
}

void Plutonium::TextBox::SetPassword(char32 replacement)
{
	if (replacement == passRepl) return;

	passRepl = replacement;
	UpdateVisibleString();
}

void Plutonium::TextBox::SetConfirmKey(Keys key)
{
	confirm = key;
}

void Plutonium::TextBox::SetKeyHoldThreshold(float seconds)
{
	holdThreshold = seconds;
}

void Plutonium::TextBox::HandleAutoSize(void)
{
	if (GetAutoSize())
	{
		/* Get the current size of the TextBox and the string (plus it's size). */
		const Vector2 size = GetSize();
		const char32 *visual = GetVisualString();
		const size_t len = strlen(visual);

		/* Create an uniform length string to make sure the TextBox doesn't resize if the focus indicator is removed. */
		const char32 *uniformSizeText = visual;
		char32 *buffer = nullptr;
		if (!showLine)
		{
			/* Add two in size for the invisible pipe character and the null terminator. */
			buffer = malloca_s(char32, len + 2);
			mrgstr(visual, U" ", buffer);
			buffer[len] = U'|';
			uniformSizeText = buffer;
		}

		/* Measure the uniform length string for autosize text dimentions and free the buffer if needed.. */
		const Vector2 textDim = GetFont()->MeasureString(uniformSizeText);
		if (!showLine) freea_s(buffer);

		/* Only apply autosize if the text is not empty. */
		if (textDim != Vector2::Zero())
		{
			const Vector2 desired = clamp(textDim + GetTextOffset() * 2.0f, minSize, maxSize);
			if (desired != size) SetSize(desired);
		}
	}
}

void Plutonium::TextBox::OnGlyphInput(WindowHandler, uint32 key)
{
	/* Check if the glyph is meant for this text box. */
	if (!(IsEnabled() && IsFocused())) return;

	/* Get actual input glyph. */
	char32 glyph = static_cast<char32>(key);
	size_t newLen = strlen(GetText()) + 1;

	bool oversized = maxStringLength > 0 && newLen > maxStringLength;
	bool rejected = false;
	GlyphType type = GlyphType::Unknown;

#if defined (_WIN32)
	/* Get glyph paramters. */
	bool isChar = IsCharAlphaW(static_cast<WCHAR>(glyph));
	bool isNumber = IsCharAlphaNumericW(static_cast<WCHAR>(glyph)) && !isChar;
	bool isPunc = ispunct(static_cast<int>(glyph));
	bool isSpace = glyph == U' ' || glyph == U'\t';
	bool isSpecial = !(isChar || isNumber || isSpace || isPunc);

	/* Check if glyph needs to be rejected. */
	if (isChar && _CrtEnumCheckFlag(flags, InputFlags::NoChars))
	{
		type = GlyphType::Character;
		rejected = true;
	}
	if (isNumber && _CrtEnumCheckFlag(flags, InputFlags::NoNumbers))
	{
		type = GlyphType::Number;
		rejected = true;
	}
	if (isSpace && _CrtEnumCheckFlag(flags, InputFlags::NoSpaces))
	{
		type = GlyphType::Space;
		rejected = true;
	}
	if (isPunc && _CrtEnumCheckFlag(flags, InputFlags::NoPunctuation))
	{
		type = GlyphType::Punctuation;
		rejected = true;
	}
	if (isSpecial && !isNumber && _CrtEnumCheckFlag(flags, InputFlags::NoSpecials))
	{
		type = GlyphType::Special;
		rejected = true;
	}
#else
	LOG_WAR_ONCE("Cannot check for 32 bit character types on this platform!");
#endif

	/* If the glyph has been rejected, reject it. */
	if (oversized || rejected) GlyphRejected.Post(this, GlyphRejectionArgs(glyph, type, oversized, rejected, false));
	else
	{
		/* Create new string. */
		char32 *buffer = malloca_s(char32, newLen + 1);
		mrgstr(GetText(), U" ", buffer);
		buffer[newLen - 1] = glyph;

		/* Set new string and free temporary buffer. */
		SetText(buffer);
		freea_s(buffer);
		UpdateVisibleString();
	}
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::TextBox::UpdateVisibleString(void)
{
	/* Get string paramters. */
	const char32 *text = GetText();
	const size_t len = strlen(text);

	/* Create result buffer. */
	char32 *buffer = malloca_s(char32, showLine ? (len + 2) : (len + 1));

	/* Populate result buffer. */
	if (passRepl == U'\0')
	{
		for (size_t i = 0; i < len; i++) buffer[i] = text[i];
	}
	else
	{
		for (size_t i = 0; i < len; i++) buffer[i] = passRepl;
	}

	/* Add pipe character to show the control is focused is needed. */
	if (showLine)
	{
		buffer[len] = U'|';
		buffer[len + 1] = U'\0';
	}
	else buffer[len] = U'\0';

	/* Set visual string and free temporary buffer. */
	SetVisualString(buffer);
	freea_s(buffer);
}
#pragma warning(pop)