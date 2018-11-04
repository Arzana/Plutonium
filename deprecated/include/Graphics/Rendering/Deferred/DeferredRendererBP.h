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
		Lighting,
		/* Renders the scene with a mask showing the shadow map used by the fragment. */
		Shadows
	};

	/* Defines a Blinn-Phong style deferred rendering handler. */
	class DeferredRendererBP
	{
	public:
		/* Defines the exposure value used to render the sceen. */
		float Exposure;
		/* 
		Defines a value (between zero and one) that will determine the mix between logarithmic and linear cascade split distances.
		Zero being only linear and one being only logarithmic.
		*/
		float CascadeLambda;
		/*
		Defines the offset (in the direction of the light) given to directional light sources when rending their shadows.
		Lower values mean that the shadows quality will be better but it might clip objects close to the center and larger shadows might not be taken into account.
		*/
		float LightOffset;
		/* Defines how this renderer should render. */
		RenderType DisplayType;

		/* Initializes a new instance of a Blinn-Phong deferred rendering handler. */
		DeferredRendererBP(_In_ Game *game, _In_opt_ bool fxaa = true, _In_opt_ bool softShadows = true);
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
		static constexpr size_t CASCADE_CNT = 3;

		std::vector<const StaticObject*> queuedModels;
		std::vector<const DynamicObject*> queuedAnimations;
		std::vector<const DirectionalLight*> queuedDLights;
		std::vector<const PointLight*> queuePLights;
		Game *game;

		RenderTarget *gbFbo, *hdrFbo, *dshdFbo;
		const RenderTargetAttachment *normalSpec;				// [nx, ny, nz, s]	G-Buffer
		const RenderTargetAttachment *posSpec;					// [x, y, z, s]		G-Buffer
		const RenderTargetAttachment *ambient;					// [r, g, b]		G-Buffer
		const RenderTargetAttachment *diffuse;					// [r, g, b]		G-Buffer
		const RenderTargetAttachment *screen;					// [r, g, b, a]		HDR-Buffer
		const RenderTargetAttachment *cascades[CASCADE_CNT];	// [d]				Shadow depth buffer (directional light).

		Buffer *plane;
		Mesh *sphere;
		MaterialBP *defMaterial;

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matview, *matMdl, *specExp;
			Uniform *mapAmbi, *mapDiff, *mapSpec, *mapAlpha, *mapBump;
			Attribute *pos, *norm, *tan, *uv;
		} gspass;		// Static Geometry pass.

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matView, *matMdl, *specExp, *amnt;
			Uniform *mapAmbi, *mapDiff, *mapSpec, *mapAlpha, *mapBump;
			Attribute *pos, *norm, *tan, *uv;
			Attribute *pos2, *norm2, *tan2;
		} gdpass;		// Dynamic Geometry pass.

		struct
		{
			Shader *shdr;
			Uniform *matLs, *matMdl, *mapAlpha, *mapDiff, *amnt;
			Attribute *pos1, *pos2, *uv;
		} dspass;		// Directional Light Shadow Pass. 

		struct 
		{
			Shader *shdr;
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *dir, *clrAmbi, *clrDiff, *clrSpec, *matView;
			Uniform *matCasc1, *matCasc2, *matCasc3, *multCasc;
			Uniform *end1, *end2, *end3;
			Uniform *shdw1, *shdw2, *shdw3;
			Attribute *pos, *uv;
		} dpass;		// Directional Light Pass.

		struct
		{
			Shader *shdr;
			Uniform *matProj, *matView, *matMdl, *screen;
			Uniform *normSpec, *ambi, *diff, *posSpec, *camPos;
			Uniform *lpos, *c, *l, *q, *clrAmbi, *clrDiff, *clrSpec;
			Attribute *pos;
		} ppass;		// Point Light Pass.

		struct
		{
			Shader *shdr;
			Uniform *screen, *gamma, *exposure, *texelStep;
			Attribute *pos, *uv;
		} fpass;		// Fix Values Pass.

		struct
		{
			Shader *shdr;
			Uniform *matMdl, *matView, *matProj, *clr, *amnt, *mapDiff;
			Attribute *pos, *pos2, *uv;
		} wire;			// Debug WireFrame Pass.

		struct
		{
			Shader *shdr;
			Uniform *normSpec;
			Attribute *pos, *uv;
		} wn;			// Debug World Normal Pass.

		struct
		{
			Shader *shdr;
			Uniform *diff;
			Attribute *pos, *uv;
		} al;			// Debug Albedo Pass.

		void InitMeshes(void);		// Plane / sphere.
		void InitGsPass(void);		// Static geometry.
		void InitGdPass(void);		// Animated geometry.
		void InitDsPass(void);		// Directional light shadow.
		void InitDPass(bool pcf);	// Directional light.
		void InitPPass(void);		// Point light.
		void InitFPass(bool fxaa);	// Monitor fix.
		void InitWire(void);		// Wireframe mode.
		void InitWNormal(void);		// World normals mode.
		void InitAlbedo(void);		// Albedo mode.

		void RenderNormal(const Camera *cam);
		void RenderWireframe(const Matrix &projection, const Matrix &view);
		void RenderWorldNormals(void);
		void RenderAlbedo(void);

		void BeginGsPass(const Matrix &proj, const Matrix &view);
		void BeginGdPass(const Matrix &proj, const Matrix &view);
		void BeginDirShadowPass(void);
		void BeginDirLightPass(Vector3 camPos);
		void BeginPntLightPass(const Matrix &proj, const Matrix &view, Vector3 camPos);

		void SetCascadeMax(const Camera *cam, float max[CASCADE_CNT]);
		float GetFurthestZFromCam(const Camera *cam);
		void SetCascadeEnds(const Camera *cam, std::vector<float> *ends);
		Box CalcOrthoBox(const Camera *cam, float near, float far);
		Matrix CalcDirLightVP(const Camera * cam, const Box &frustum, const DirectionalLight *light);

		void RenderModel(const Camera *cam, const StaticObject *model, const MaterialBP *overrideMaterial);
		void RenderModel(const Camera *cam, const DynamicObject *model, const MaterialBP *overrideMaterial);
		void RenderDirLightShadow(const Camera *cam, const DirectionalLight *light, Matrix *spaces, std::vector<float> *ends);
		void RenderDirLight(const Matrix &view, const DirectionalLight *light, Matrix *spaces, std::vector<float> *ends);
		void RenderPntLight(const PointLight *light);
		void FixForMonitor(void);
	};
}