#pragma once
#include "Graphics\Renderer.h"
#include "GameLogic\StaticObject.h"
#include "Graphics\Lighting\DirectionalLight.h"
#include "Graphics\Lighting\PointLight.h"
#include "Graphics\GraphicsAdapter.h"

namespace Plutonium
{
	/* Defines a very basic model renderer. */
	struct StaticRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of a basic model renderer. */
		StaticRenderer(_In_ const GraphicsAdapter *device, _In_ const char *vrtxShdr, _In_ const char * fragShdr);
		StaticRenderer(_In_ const StaticRenderer &value) = delete;
		StaticRenderer(_In_ StaticRenderer &&value) = delete;

		_Check_return_ StaticRenderer& operator =(_In_ const StaticRenderer &other) = delete;
		_Check_return_ StaticRenderer& operator =(_In_ StaticRenderer &&other) = delete;

		/* Starts rendering the specified scene. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ Vector3 camPos, _In_ const DirectionalLight *sun, _In_ const PointLight *pointLight[4]);
		/* Renders the specified model. */
		void Render(_In_ const StaticObject *model);

	private:
		Uniform * matMdl, *matView, *matProj, *camPos, *specExp, *gamma;
		Uniform *mapAmbi, *mapDiff, *mapSpec, *mapAlpha, *mapBump;
		Uniform *sunLightDir, *sunLightAmbi, *sunLightDiff, *sunLightSpec; 
		Uniform *pointLightPos[4], *pointLightAtten[4], *pointLightAmbi[4], *pointLightDiff[4], *pointLightSpec[4];
		Attribute *pos, *norm, *tan, *uv;
		const GraphicsAdapter *device;
	};
}