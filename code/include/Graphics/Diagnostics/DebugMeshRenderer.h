#pragma once
#include "Graphics\Diagnostics\ViewModes\WireframeRenderer.h"
#include "Graphics\Diagnostics\ViewModes\NormalRenderer.h"
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
		Normals
	};

	/* Defines a very basic debug mesh information renderer. */
	struct DebugMeshRenderer
	{
	public:
		/* Initializes a new instance of a basic mesh renderer. */
		DebugMeshRenderer(_In_ WindowHandler wnd, _In_opt_ DebuggableValues mode = DebuggableValues::Wireframe);
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

		/* Start rendering the specified scene. */
		void Begin(_In_ const Matrix &view, const Matrix &proj);
		/* Renders the specified model as a wireframe. */
		void Render(_In_ const StaticObject *model, _In_opt_ Color color = Color::Red);
		/* Render the specified model as a wireframe. */
		void Render(_In_ const DynamicObject *model, _In_opt_ Color color = Color::Yellow);
		/* Stops rendering the specified scene. */
		void End(void);

	private:
		DebuggableValues mode;
		WireframeRenderer *wfrenderer;
		NormalRenderer *nrenderer;
		Texture *defBmpMap;

		void RenderWfStatic(const StaticObject *model, Color color);
		void RenderWfDynamic(const DynamicObject *model, Color color);
		void RenderNStatic(const StaticObject *model);
		void RenderNDynamic(const DynamicObject *model);
	};
}