#pragma once
#include "Graphics\Native\Monitor.h"
#include "Graphics\Native\Window.h"
#include "Graphics\Native\BlendState.h"
#include "Graphics\Native\BlendType.h"
#include "Graphics\Native\FaceCullState.h"
#include "Graphics\Native\FaceCullType.h"
#include "Graphics\Native\DepthState.h"

namespace Plutonium
{
	/* Defines the global graphics settings. */
	struct GraphicsAdapter
	{
	public:
		GraphicsAdapter(_In_ const GraphicsAdapter &value) = delete;
		GraphicsAdapter(_In_ GraphicsAdapter &&value) = delete;
		/* Releases the window associated with the graphics. */
		~GraphicsAdapter(void) noexcept;

		_Check_return_ GraphicsAdapter& operator =(_In_ const GraphicsAdapter &other) = delete;
		_Check_return_ GraphicsAdapter& operator =(_In_ GraphicsAdapter &&other) = delete;

		/* Sets the alpha blend function. */
		void SetAlphaBlendFunction(_In_ BlendState func);
		/* Sets the color blend function. */
		void SetColorBlendFunction(_In_ BlendState func);
		/* Sets the source alpha blend type. */
		void SetAlphaSourceBlend(_In_ BlendType blend);
		/* Sets the destination alpha blend type. */
		void SetAlphaDestinationBlend(_In_ BlendType blend);
		/* Sets the source color blend type. */
		void SetColorSourceBlend(_In_ BlendType blend);
		/* Sets the destination color blend type. */
		void SetColorDestinationBlend(_In_ BlendType blend);

		/* Sets the way faces should be culled. */
		void SetFaceCull(_In_ FaceCullState cull);
		/* Sets the way front facing faces are determined. */
		void SetFrontFace(_In_ FaceCullType func);

		/* Sets the depth testing. */
		void SetDepthTest(_In_ DepthState func);

		/* Gets the window associated with the context. */
		_Check_return_ inline Window* GetWindow(void) const
		{
			return window;
		}

	private:
		friend struct Game;

		Window *window;
		BlendState abf, cbf;

		GraphicsAdapter(Window *window);

		void UpdateBlendEq(BlendState func);
	};
}