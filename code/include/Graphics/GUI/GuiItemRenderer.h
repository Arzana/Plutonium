#pragma once
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\Buffer.h"
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

		/* Adds a basic GuiItem to the render queue. */
		void RenderGuiItem(_In_ Rectangle bounds, _In_ float rounding, _In_ float orientation, _In_ Color backColor, _In_ TextureHandler background, _In_ bool shouldDrawResizer, _In_ const Buffer *mesh);
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
			float Orientation;
			float Rounding;
			Color BackgroundColor;
			Color ResizerColor;
			TextureHandler Background;
		};

		struct
		{
			Shader *shdr;
			Uniform *matMdl, *matProj;
			Uniform *clrBack, *clrResiz, *background;
			Uniform *rounding, *pos, *size;
			Attribute *posUv;
		} basic;
		
		Matrix projection;
		Texture *defBackTex;
		std::queue<BasicGuiItemArgs> basicDrawQueue;

		void RenderBasics(void);
		void WindowResizeEventHandler(WindowHandler sender, EventArgs args);
		void InitBasicShader(void);
	};
}