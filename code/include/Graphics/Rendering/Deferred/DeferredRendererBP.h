#pragma once
#include "Game.h"
#include "Components\Camera.h"
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\RenderTargets\RenderTarget.h"
#include "GameLogic\StaticObject.h"
#include "GameLogic\DynamicObject.h"
#include "Graphics\Lighting\DirectionalLight.h"
#include "Graphics\Lighting\PointLight.h"

namespace Plutonium
{
	/* Defines the ways the DeferredRenderer can render. */
	enum class RenderType
	{
		/* Renders the scene normally. */
		Normal,
		/* Renders the scene as wireframes. */
		Wireframe,
		/* Renders the scene's world normals visible. */
		WorldNormals,
		/* Renders only the albedo of the materials. */
		Albedo,
		/* Renders the scene with default materials. */
		Lighting
	};

	/* Defines a Blinn-Phong style deferred rendering handler. */
	struct DeferredRendererBP
	{
	public:
		/* Defines the exposure value used to render the sceen. */
		float Exposure;
		/* Defines how this renderer should render. */
		RenderType DisplayType;

		/* Initializes a new instance of a Blinn-Phong deferred rendering handler. */
		DeferredRendererBP(_In_ Game *game);
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
		void Render(_In_ const Camera *cam);

	private:
		std::vector<const StaticObject*> queuedModels;
		std::vector<const DynamicObject*> queuedAnimations;
		std::vector<const DirectionalLight*> queuedDLights;
		std::vector<const PointLight*> queuePLights;
		Game *game;

		RenderTarget *gbFbo, *hdrFbo, *dshdFbo;
		const RenderTargetAttachment *normalSpec;	// [nx, ny, nz, s]	G-Buffer
		const RenderTargetAttachment *posSpec;		// [x, y, z, s]		G-Buffer
		const RenderTargetAttachment *ambient;		// [r, g, b]		G-Buffer
		const RenderTargetAttachment *diffuse;		// [r, g, b]		G-Buffer
		const RenderTargetAttachment *screen;		// [r, g, b, a]		HDR-Buffer
		const RenderTargetAttachment *shadow;		// [d]				Shadow depth buffer (directional light).

		Buffer *plane;
		Mesh *sphere;
		MaterialBP *defMaterial;

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
			Uniform *matLs, *matMdl, *mapAlpha, *mapDiff, *amnt;
			Attribute *pos1, *pos2, *uv;
		} dspass;

		struct 
		{
			Shader *shdr;
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *dir, *clrAmbi, *clrDiff, *clrSpec, *matView, *mapShdw;
			Attribute *pos, *uv;
		} dpass;

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matView, *matMdl, *screen;
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *lpos, *c, *l, *q, *clrAmbi, *clrDiff, *clrSpec;
			Attribute *pos;
		} ppass;

		struct
		{
			Shader *shdr;
			Uniform *screen, *gamma, *exposure;
			Attribute *pos, *uv;
		} fpass;

		struct
		{
			Shader *shdr;
			Uniform *matMdl, *matView, *matProj, *clr, *amnt, *mapDiff;
			Attribute *pos, *pos2, *uv;
		} wire;

		struct
		{
			Shader *shdr;
			Uniform *normSpec;
			Attribute *pos, *uv;
		} wn;

		struct
		{
			Shader *shdr;
			Uniform *diff;
			Attribute *pos, *uv;
		} al;

		void InitMeshes(void);	// Plane / sphere.
		void InitGsPass(void);	// Static geometry.
		void InitGdPass(void);	// Animated geometry.
		void InitDsPass(void);	// Directional light shadow.
		void InitDPass(void);	// Directional light.
		void InitPPass(void);	// Point light.
		void InitFPass(void);	// Monitor fix.
		void InitWire(void);	// Wireframe mode.
		void InitWNormal(void);	// World normals mode.
		void InitAlbedo(void);	// Albedo mode.

		void RenderNormal(const Camera *cam);
		void RenderWireframe(const Matrix &projection, const Matrix &view);
		void RenderWorldNormals(void);
		void RenderAlbedo(void);

		void BeginGsPass(const Matrix &proj, const Matrix &view);
		void BeginGdPass(const Matrix &proj, const Matrix &view);
		void BeginDirShadowPass(void);
		void BeginDirLightPass(Vector3 camPos);
		void BeginPntLightPass(const Matrix &proj, const Matrix &view, Vector3 camPos);

		void RenderModel(const Camera *cam, const StaticObject *model, const MaterialBP *overrideMaterial);
		void RenderModel(const Camera *cam, const DynamicObject *model, const MaterialBP *overrideMaterial);
		Matrix RenderDirLightShadow(const Camera *cam, const DirectionalLight *light);
		void RenderDirLight(const Matrix &space, const DirectionalLight *light);
		void RenderPntLight(const PointLight *light);
		void FixForMonitor(void);
	};
}