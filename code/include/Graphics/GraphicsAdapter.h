#pragma once
#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Graphics\Native\Monitor.h"
#include "Graphics\Native\Window.h"
#include "Graphics\Native\RenderTarget.h"
#include "Graphics\Native\BlendState.h"
#include "Graphics\Native\BlendType.h"
#include "Graphics\Native\FaceCullState.h"
#include "Graphics\Native\FaceCullType.h"
#include "Graphics\Native\DepthState.h"
#include "Graphics\Native\StencilOperation.h"
#include "Graphics\Native\ClearTargets.h"

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
		/* Sets the operation to perform if the stencil test fails. */
		void SetStencilFailOperation(_In_ StencilOperation operation);
		/* Sets the operation to perform if the stencil test passes but the depth test fails. */
		void SetStencilPassDepthFailOperation(_In_ StencilOperation operation);
		/* Sets the operation to perform if the stencil test passes and the depth test passes. */
		void SetStencilPassDepthPassOperation(_In_ StencilOperation operation);

		/* Sets the depth testing. */
		void SetDepthTest(_In_ DepthState func);
		/* Sets the stencil testing. */
		void SetStencilTest(_In_ DepthState func, _In_ int32 value = 0, _In_ uint32 mask = 0xFF);

		/* Sets whether output to the color buffer is enabled or disabled per component. */
		void SetColorOutput(_In_ bool red, _In_ bool green, _In_ bool blue, _In_ bool alpha);
		/* Sets whether output to the depth buffer is enabled or disabled. */
		void SetDepthOuput(_In_ bool mask);
		/* Sets the output mask for the stencil buffer. */
		void SetStencilOuput(_In_ uint32 mask);

		/* Clears the specified buffers. */
		void Clear(_In_ ClearTarget target);
		/* Sets the current render target (nullptr for window). */
		void SetRenderTarget(_In_ const RenderTarget *target);

		/* Gets the window associated with the context. */
		_Check_return_ inline Window* GetWindow(void) const
		{
			return window;
		}

		/* Gets the information of the graphics device. */
		_Check_return_ inline const DeviceInfo* GetInfo(void) const
		{
			return device;
		}

		/* Converts a 2D vector from screen space (top-left origin) to OpenGL space (bottom-left origin). */
		_Check_return_ Vector2 ToOpenGL(_In_ Vector2 screenCoord);

	private:
		friend struct Game;

		const DeviceInfo *device;
		Window *window;
		BlendState abf, cbf;
		StencilOperation sf, df, dp;

		GraphicsAdapter(Window *window);

		void SetDefaultBlendEq(void);
		void SetDefaultStencilOp(void);
		void UpdateBlendEq(BlendState func);
		void UpdateStencilOp(void);
	};
}