#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Mesh.h"
#include "Graphics\Texture.h"
#include "Graphics\Lighting\DirectionalLight.h"
#include "Graphics\Lighting\PointLight.h"

namespace Plutonium
{
	/* Defines a rendering that renders using a default material. */
	struct LightingRenderer
	{
	public:
		/* Initializes a new instance of a lighing renderer. */
		LightingRenderer(_In_ GraphicsAdapter *device);
		LightingRenderer(_In_ const LightingRenderer &value) = delete;
		LightingRenderer(_In_ LightingRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~LightingRenderer(void);

		_Check_return_ LightingRenderer& operator =(_In_ const LightingRenderer &other) = delete;
		_Check_return_ LightingRenderer& operator =(_In_ LightingRenderer &&other) = delete;

		/* Starts the mesh renderer. */
		void BeginModels(_In_ const Matrix &view, _In_ const Matrix &proj);
		/* Starts the directional lighting renderer. */
		void BeginDirectionalLights(_In_ Vector3 camPos);
		/* Starts the directional lighting renderer. */
		void BeginPointLights(void);
		/* Renders a single shape with the neutral material. */
		void Render(_In_ const Matrix &world, _In_ const Mesh *mesh);
		/* Renders a single directional light in the scene. */
		void Render(_In_ const Matrix &world, _In_ const Mesh *mesh, _In_ Texture *bumpMap, _In_ const DirectionalLight *light);
		/* Renders a single point light in the scene. */
		void Render(_In_ const Matrix &world, _In_ const Mesh *mesh, _In_ Texture *bumpMap, _In_ const PointLight *light);
		/* Ends the renderer. */
		void End(void);

	private:
		Shader *shdrMesh, *shdrDLight, *shdrPLight;

		GraphicsAdapter *device;
		Matrix view, proj;
		Vector3 camPos;
		byte state;

		Uniform *matMdlM, *matViewM, *matProjM;
		Attribute *posM;

		Uniform *matMdlDl, *matViewDl, *matProjDl, *camPosDl, *mapBmpDl;
		Uniform *ambiDl, *diffDl, *specDl, *dirDl;
		Attribute *posDl, *normDl, *tanDl, *uvDl;

		Uniform *matMdlPl, *matViewPl, *matProjPl, *camPosPl, *mapBmpPl;
		Uniform *ambiPl, *diffPl, *specPl, *lposPl, *attenPl;
		Attribute *posPl, *normPl, *tanPl, *uvPl;
	};
}