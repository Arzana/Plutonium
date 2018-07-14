#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\RenderTargets\RenderTarget.h"
#include "GameLogic\StaticObject.h"
#include "Graphics\Lighting\DirectionalLight.h"
#include "Graphics\Lighting\PointLight.h"

namespace Plutonium
{
	/* Defines a Blinn-Phong style deferred style rendering handler. */
	struct DeferredRendererBP
	{
	public:
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
		/* Adds a directional light to the render queue. */
		void Add(_In_ const DirectionalLight *light);
		/* Adds a point light to the render queue. */
		void Add(_In_ const PointLight *light);
		/* Renders the scene. */
		void Render(_In_ const Matrix &projection, _In_ const Matrix &view, _In_ Vector3 camPos);

	private:
		std::queue<const StaticObject*> queuedModels;
		std::queue<const DirectionalLight*> queuedDLights;
		std::queue<const PointLight*> queuePLights;
		GraphicsAdapter *device;

		RenderTarget *fbo;
		const RenderTargetAttachment *normalSpec;	// [nx, ny, nz, s]
		const RenderTargetAttachment *posSpec;		// [x, y, z, s]
		const RenderTargetAttachment *ambient;		// [r, g, b]
		const RenderTargetAttachment *diffuse;		// [r, g, b]

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matview, *matMdl, *specExp;
			Uniform *mapAmbi, *mapDiff, *mapSpec, *mapAlpha, *mapBump;
			Attribute *pos, *norm, *tan, *uv;
		} gpass;

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
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *lpos, *atten, *clrAmbi, *clrDiff, *clrSpec;
			Attribute *pos, *uv;
		} ppass;

		void InitGPass(void);
		void InitDPass(void);
		void InitPPass(void);

		void BeginGPass(const Matrix &proj, const Matrix &view);
		void RenderModel(const StaticObject *model);
		void EndGPass(void);
		void BeginDirLightPass(Vector3 camPos);
		void RenderDirLight(const DirectionalLight *light);
		void EndDirLightPass(void);
		void BeginPntLightPass(Vector3 camPos);
		void RenderPntLight(const PointLight *light);
		void EndPntLightPass(void);
	};
}