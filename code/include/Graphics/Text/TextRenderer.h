#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\GraphicsAdapter.h"
#include "Font.h"
#include "Graphics\Native\Buffer.h"
#include <vector>

namespace Plutonium
{
	/* Defines a helper object for rendering text. */
	struct FontRenderer
	{
	public:
		/* Initializes a new instance of a font renderer. */
		FontRenderer(_In_ GraphicsAdapter *device, _In_ const char *font, _In_ const char *vrtxShdr, _In_ const char *fragShdr);
		FontRenderer(_In_ const FontRenderer &value) = delete;
		FontRenderer(_In_ FontRenderer &&value) = delete;
		/* Releases the resources allocated by the font renderer. */
		~FontRenderer(void);

		_Check_return_ FontRenderer& operator =(_In_ const FontRenderer &other) = delete;
		_Check_return_ FontRenderer& operator =(_In_ FontRenderer &&other) = delete;

		/* Adds a string to the font renderer to be rendered in the next frame. */
		void AddString(_In_ Vector2 pos, _In_ const char *str, _In_ Color clr = Color::White);
		/* Renders the strings specified throughout the frame. */
		virtual void Render(void);

		/* Gets the font used by this renderer. */
		_Check_return_ inline const Font* GetFont(void) const
		{
			return font;
		}

	protected:
		/* Defines the rendering attributes for a string. */
		struct LineInfo
		{
			/* The string to render, */
			const char *String;
			/* Where to render the string(top-left). */
			Vector2 Position;
			/* The color of the string. */
			Color Color;

			/* Initializes a new instance. */
			LineInfo(_In_ const char *string, _In_ Vector2 pos, _In_ Plutonium::Color clr);
			LineInfo(_In_ const LineInfo &value) = delete;
			LineInfo(_In_ LineInfo &&value) = delete;
			/* Releases the underlying string. */
			~LineInfo(void) noexcept;

			_Check_return_ LineInfo& operator =(_In_ const LineInfo &other) = delete;
			_Check_return_ LineInfo& operator =(_In_ LineInfo &&other) = delete;
		};

		/* The font used by the renderer. */
		Font *font;
		/* A buffer for storing the strings. */
		std::vector<LineInfo*> strs;
		/* The window associated with the renderer. */
		GraphicsAdapter *device;

	private:
		Buffer *vbo;

		Shader *shdr;
		Uniform *clr;
		Uniform *tex;
		Uniform *wvp;
		Attribute *pos;
		Matrix proj;

		void RenderString(LineInfo *info);
		void UpdateVBO(Vector2 pos, const char *str);
		void AddSingleString(Vector2 pos, const char *str, Color clr);
		void WindowResizeEventHandler(WindowHandler sender, EventArgs args);
		void ClearBuffer(void);
	};
}