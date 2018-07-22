#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\RenderTargets\RenderTarget.h"
#include "GameLogic\StaticObject.h"
#include "GameLogic\DynamicObject.h"
#include "Graphics\Lighting\DirectionalLight.h"
#include "Graphics\Lighting\PointLight.h"

namespace Plutonium
{
	/* Defines a Blinn-Phong style deferred style rendering handler. */
	struct DeferredRendererBP
	{
	public:
		/* Defines the exposure value used to render the sceen. */
		float Exposure;

		/* Initializes a new instance of a Blinn-Phong deferred rendering handler. */
		DeferredRendererBP(_In_ GraphicsAdapter *device);
		DeferredRendererBP(_In_ const DeferredRendererBP &value) = delete;
		DeferredRendererBP(_In_ DeferredRendererBP &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~DeferredRendererBP(void);

		_Check_return_ DeferredRendererBP& operator =(_In_ const DeferredRendererBP &other) = delete;
		_Check_return_ DeferredRendererBP& operator =(_In_ DeferredRendererBP &&other) = delete;

		/* Adds a static model to the render queue. */
		void Add(_In_ const StaticObject *model);
		/* Adds a animated model to the render queue. */
		void Add(_In_ const DynamicObject *model);
		/* Adds a directional light to the render queue. */
		void Add(_In_ const DirectionalLight *light);
		/* Adds a point light to the render queue. */
		void Add(_In_ const PointLight *light);
		/* Renders the scene. */
		void Render(_In_ const Matrix &projection, _In_ const Matrix &view, _In_ Vector3 camPos);

	private:
		std::queue<const StaticObject*> queuedModels;
		std::queue<const DynamicObject*> queuedAnimations;
		std::queue<const DirectionalLight*> queuedDLights;
		std::queue<const PointLight*> queuePLights;
		GraphicsAdapter *device;

		RenderTarget *gbFbo, *hdrFbo;
		const RenderTargetAttachment *normalSpec;	// [nx, ny, nz, s]	G-Buffer
		const RenderTargetAttachment *posSpec;		// [x, y, z, s]		G-Buffer
		const RenderTargetAttachment *ambient;		// [r, g, b]		G-Buffer
		const RenderTargetAttachment *diffuse;		// [r, g, b]		G-Buffer
		const RenderTargetAttachment *screen;		// [r, g, b, a]		HDR-Buffer

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matview, *matMdl, *specExp;
			Uniform *mapAmbi, *mapDiff, *mapSpec, *mapAlpha, *mapBump;
			Attribute *pos, *norm, *tan, *uv;
		} gspass;

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matView, *matMdl, *specExp, *amnt;
			Uniform *mapAmbi, *mapDiff, *mapSpec, *mapAlpha, *mapBump;
			Attribute *pos, *norm, *tan, *uv;
			Attribute *pos2, *norm2, *tan2;
		} gdpass;

		struct 
		{
			Shader *shdr;
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *dir, *clrAmbi, *clrDiff, *clrSpec;
			Attribute *pos, *uv;

			Buffer *plane;
		} dpass;

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matView, *matMdl, *screen;
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *lpos, *c, *l, *q, *clrAmbi, *clrDiff, *clrSpec;
			Attribute *pos;

			Mesh *sphere;
		} ppass;

		struct
		{
			Shader *shdr;
			Uniform *screen, *gamma, *exposure;
			Attribute *pos, *uv;
		} fpass;

		void InitGsPass(void);	// Static geometry.
		void InitGdPass(void);	// Animated geometry.
		void InitDPass(void);	// Directional light.
		void InitPPass(void);	// Point light.
		void InitFPass(void);	// Monitor fix.

		void BeginGsPass(const Matrix &proj, const Matrix &view);
		void BeginGdPass(const Matrix &proj, const Matrix &view);
		void BeginDirLightPass(Vector3 camPos);
		void BeginPntLightPass(const Matrix &proj, const Matrix &view, Vector3 camPos);

		void RenderModel(const StaticObject *model);
		void RenderModel(const DynamicObject *model);
		void RenderDirLight(const Matrix &iview, const DirectionalLight *light);
		void RenderPntLight(const PointLight *light);
		void FixForMonitor(void);
	};
}