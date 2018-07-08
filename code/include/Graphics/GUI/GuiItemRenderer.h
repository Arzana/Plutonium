#pragma once
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\Buffer.h"
#include "Graphics\Text\Font.h"
#include <queue>

namespace Plutonium
{
	/* Defines a renderer for all GuiItems. */
	struct GuiItemRenderer
	{
	public:
		/* Initializes a new instance of a GuiItem renderer with a specified graphics device. */
		GuiItemRenderer(_In_ GraphicsAdapter *device);
		GuiItemRenderer(_In_ const GuiItemRenderer &value) = delete;
		GuiItemRenderer(_In_ GuiItemRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~GuiItemRenderer(void);

		_Check_return_ GuiItemRenderer& operator =(_In_ const GuiItemRenderer &other) = delete;
		_Check_return_ GuiItemRenderer& operator =(_In_ GuiItemRenderer &&other) = delete;

		/* Adds a basic item to the render queue. */
		void RenderBackground(_In_ Rectangle bounds, _In_ float rounding, _In_ Color backColor, _In_ TextureHandler background, _In_ const Buffer *mesh, _In_ bool lateCall);
		/* Adds the Label text foregroung to the render queue. */
		void RenderTextForeground(_In_ Vector2 position, _In_ Color textColor, _In_ const Font *font, _In_ const char32 *text, _In_ const Buffer *mesh);
		/* Adds the ProgressBar bar section to the render queue. */
		void RenderBarForeground(_In_ Vector2 position, _In_ Rectangle parentBounds, _In_ float parentRounding, _In_ Color barColor, _In_ TextureHandler texture, const Buffer *mesh);
		/* Renders the queued GuiItems to the screen. */
		void End(_In_opt_ bool noBlending = false);

	protected:
		GraphicsAdapter *device;

	private:
		struct BasicGuiItemArgs
		{
			const Buffer *Mesh;
			Vector2 Position;
			Vector2 Size;
			float Rounding;
			Color BackgroundColor;
			TextureHandler Background;
		};

		struct LabelTextArgs
		{
			const Buffer *Mesh;
			const Font *Font;
			Vector2 Position;
			Color TextColor;
			const char32 *Text;
		};

		struct ProgressBarBarArgs
		{
			const Buffer *Mesh;
			Vector2 Position;
			Vector2 ParentPosition;
			Vector2 ParentSize;
			float ParentRounding;
			Color BarColor;
			TextureHandler Texture;
		};

		struct
		{
			Shader *shdr;
			Uniform *matMdl, *matProj;
			Uniform *clrBack, *background;
			Uniform *rounding, *pos, *size;
			Attribute *posUv;
		} basic;

		struct
		{
			Shader *shdr;
			Uniform *matMdl, *matProj;
			Uniform *map, *clr;
			Attribute *posUv;
		} text;

		struct
		{
			Shader *shdr;
			Uniform *matMdl, *matProj;
			Uniform *texture, *clr, *rounding, *size, *pos;
			Attribute *posUv;
		} bar;
		
		Matrix projection;
		Texture *defBackTex;
		std::queue<BasicGuiItemArgs> basicEarlyDrawQueue;
		std::queue<BasicGuiItemArgs> basicLateDrawQueue;
		std::queue<LabelTextArgs> textDrawQueue;
		std::queue<ProgressBarBarArgs> barDrawQueue;

		void RenderBasics(std::queue<BasicGuiItemArgs> &queue);
		void RenderText(void);
		void RenderBars(void);
		void WindowResizeEventHandler(WindowHandler sender, EventArgs args);
		void InitBasicShader(void);
		void InitTextShader(void);
		void InitBarShader(void);
	};
}