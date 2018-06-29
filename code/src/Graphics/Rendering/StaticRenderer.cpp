#include "Graphics\Rendering\StaticRenderer.h"

Plutonium::StaticRenderer::StaticRenderer(const char * vrtxShdr, const char * fragShdr, float displayGamma)
	: Renderer(Shader::FromFile(vrtxShdr, fragShdr)), gammaValue(displayGamma)
{
	const Shader *shdr = GetShader();

	/* Get uniforms. */
	matMdl = shdr->GetUniform("u_transform.model");
	matView = shdr->GetUniform("u_transform.view");
	matProj = shdr->GetUniform("u_transform.projection");
	camPos = shdr->GetUniform("u_view_pos");

	mapAmbi = shdr->GetUniform("u_textures.ambient");
	mapDiff = shdr->GetUniform("u_textures.diffuse");
	mapSpec = shdr->GetUniform("u_textures.specular");
	mapAlpha = shdr->GetUniform("u_textures.alpha");
	mapBump = shdr->GetUniform("u_textures.bump");

	filter = shdr->GetUniform("u_colors.lfilter");
	ambient = shdr->GetUniform("u_colors.ambient");
	diffuse = shdr->GetUniform("u_colors.diffuse");
	specular = shdr->GetUniform("u_colors.specular");
	specExp = shdr->GetUniform("u_colors.specularExponent");
	gamma = shdr->GetUniform("u_colors.displayGamma");

	sunLightDir = shdr->GetUniform("u_light_sun.direction");
	sunLightAmbi = shdr->GetUniform("u_light_sun.ambient");
	sunLightDiff = shdr->GetUniform("u_light_sun.diffuse");
	sunLightSpec = shdr->GetUniform("u_light_sun.specular");

	pointLightPos[0] = shdr->GetUniform("u_light_vases[0].position");
	pointLightAtten[0] = shdr->GetUniform("u_light_vases[0].attenuation");
	pointLightAmbi[0] = shdr->GetUniform("u_light_vases[0].ambient");
	pointLightDiff[0] = shdr->GetUniform("u_light_vases[0].diffuse");
	pointLightSpec[0] = shdr->GetUniform("u_light_vases[0].specular");
	pointLightPos[1] = shdr->GetUniform("u_light_vases[1].position");
	pointLightAtten[1] = shdr->GetUniform("u_light_vases[1].attenuation");
	pointLightAmbi[1] = shdr->GetUniform("u_light_vases[1].ambient");
	pointLightDiff[1] = shdr->GetUniform("u_light_vases[1].diffuse");
	pointLightSpec[1] = shdr->GetUniform("u_light_vases[1].specular");
	pointLightPos[2] = shdr->GetUniform("u_light_vases[2].position");
	pointLightAtten[2] = shdr->GetUniform("u_light_vases[2].attenuation");
	pointLightAmbi[2] = shdr->GetUniform("u_light_vases[2].ambient");
	pointLightDiff[2] = shdr->GetUniform("u_light_vases[2].diffuse");
	pointLightSpec[2] = shdr->GetUniform("u_light_vases[2].specular");
	pointLightPos[3] = shdr->GetUniform("u_light_vases[3].position");
	pointLightAtten[3] = shdr->GetUniform("u_light_vases[3].attenuation");
	pointLightAmbi[3] = shdr->GetUniform("u_light_vases[3].ambient");
	pointLightDiff[3] = shdr->GetUniform("u_light_vases[3].diffuse");
	pointLightSpec[3] = shdr->GetUniform("u_light_vases[3].specular");

	/* Get attributes. */
	pos = shdr->GetAttribute("a_position");
	norm = shdr->GetAttribute("a_normal");
	tan = shdr->GetAttribute("a_tangent");
	uv = shdr->GetAttribute("a_uv");
}

void Plutonium::StaticRenderer::Begin(const Matrix & view, const Matrix & proj, Vector3 camPos, const DirectionalLight * sun, const PointLight * pointLight[4])
{
	Renderer::Begin();

	/* Set constant uniforms. */
	matView->Set(view);
	matProj->Set(proj);
	gamma->Set(gammaValue);
	this->camPos->Set(camPos);

	sunLightDir->Set(sun->Direction);
	sunLightAmbi->Set(sun->Ambient);
	sunLightDiff->Set(sun->Diffuse);
	sunLightSpec->Set(sun->Specular);

	for (size_t i = 0; i < 4; i++)
	{
		pointLightPos[i]->Set(view * pointLight[i]->Position);
		pointLightAtten[i]->Set(Vector3(pointLight[i]->Constant, pointLight[i]->Linear, pointLight[i]->Quadratic));
		pointLightAmbi[i]->Set(pointLight[i]->Ambient);
		pointLightDiff[i]->Set(pointLight[i]->Diffuse);
		pointLightSpec[i]->Set(pointLight[i]->Specular);
	}
}

void Plutonium::StaticRenderer::Render(const StaticObject * model)
{
	matMdl->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->GetModel()->shapes.size(); i++)
	{
		/* Get current textured mesh. */
		PhongMaterial *cur = model->GetModel()->shapes.at(i);
		Buffer *buffer = cur->Mesh->GetVertexBuffer();

		/* Set material attributes. */
		mapAmbi->Set(cur->AmbientMap);
		mapDiff->Set(cur->DiffuseMap);
		mapSpec->Set(cur->SpecularMap);
		mapAlpha->Set(cur->AlphaMap);
		mapBump->Set(cur->BumpMap);
		specExp->Set(cur->SpecularExp);
		filter->Set(cur->Transmittance);
		ambient->Set(cur->Ambient);
		diffuse->Set(cur->Diffuse);
		specular->Set(cur->Specular);

		/* Set mesh buffer attributes. */
		buffer->Bind();
		pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
		norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
		tan->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
		uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render current shape. */
		DrawTris(buffer);
	}
}