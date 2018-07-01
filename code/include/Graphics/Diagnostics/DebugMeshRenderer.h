#pragma once
#include "Graphics\Diagnostics\ViewModes\WireframeRenderer.h"
#include "Graphics\Diagnostics\ViewModes\NormalRenderer.h"
#include "Graphics\Diagnostics\ViewModes\UnlitRenderer.h"
#include "Graphics\Diagnostics\ViewModes\LightingRenderer.h"
#include "GameLogic\StaticObject.h"
#include "GameLogic\DynamicObject.h"

namespace Plutonium
{
	/* Defines the ways the DebugMeshRenderer can render meshes. */
	enum class DebuggableValues
	{
		/* This value should not be used! */
		None,
		/* Displays the models wireframe. */
		Wireframe,
		/* Displays the models normals. */
		Normals,
		/* Displays the models without lighting. */
		Unlit,
		/* Displays the lighting affecting the scene with default materials. */
		Lighting
	};

	/* Defines a very basic debug mesh information renderer. */
	struct DebugMeshRenderer
	{
	public:
		/* Initializes a new instance of a basic mesh renderer. */
		DebugMeshRenderer(_In_ GraphicsAdapter *device, _In_opt_ DebuggableValues mode = DebuggableValues::Wireframe);
		DebugMeshRenderer(_In_ const DebugMeshRenderer &value) = delete;
		DebugMeshRenderer(_In_ DebugMeshRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~DebugMeshRenderer(void);

		_Check_return_ DebugMeshRenderer& operator =(_In_ const DebugMeshRenderer &other) = delete;
		_Check_return_ DebugMeshRenderer& operator =(_In_ DebugMeshRenderer &&other) = delete;

		/* Sets the render mode of the debug mesh renderer. */
		inline void SetMode(_In_ DebuggableValues mode)
		{
			this->mode = mode;
		}

		/* Adds a static model to the debug scene. */
		inline void AddModel(_In_ const StaticObject *model)
		{
			sModels.push_back(model);
		}
		/* Adds a dynamic model to the debug scene. */
		inline void AddModel(_In_ const DynamicObject *model)
		{
			dModels.push_back(model);
		}
		/* Adds a directional light to the debug scene. */
		inline void AddLight(_In_ const DirectionalLight *light)
		{
			dLights.push(light);
		}
		/* Adds a point light to the debug scene. */
		inline void AddLight(_In_ const PointLight *light)
		{
			pLights.push(light);
		}

		/* Renders the debug scene. */
		void Render(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ Vector3 camPos);

	private:
		DebuggableValues mode;
		Texture *defBmpMap;
		Texture *defAlphaMap;

		WireframeRenderer *wfrenderer;
		NormalRenderer *nrenderer;
		UnlitRenderer *ulrenderer;
		LightingRenderer *lrenderer;

		std::deque<const StaticObject*> sModels;
		std::deque<const DynamicObject*> dModels;
		std::queue<const DirectionalLight*> dLights;
		std::queue<const PointLight*> pLights;

		void RenderWfStatic(const StaticObject *model, Color color);
		void RenderWfDynamic(const DynamicObject *model, Color color);
		void RenderNStatic(const StaticObject *model);
		void RenderNDynamic(const DynamicObject *model);
		void RenderUlStatic(const StaticObject *model);
		void RenderUlDynamic(const DynamicObject *model);
		void RenderLStatic(const StaticObject *model);
		void RenderLDynamic(const DynamicObject *model);
		void RenderLDLight(const DirectionalLight *light);
		void RenderLPLight(const PointLight *light);
	};
}