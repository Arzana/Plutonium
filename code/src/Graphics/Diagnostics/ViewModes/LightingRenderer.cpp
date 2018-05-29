#include "Graphics\Diagnostics\ViewModes\LightingRenderer.h"

constexpr const char *MESH_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"uniform mat4 model;																									\n"
"uniform mat4 view;																										\n"
"uniform mat4 projection;																								\n"

"in vec3 position;																										\n"

"void main()																											\n"
"{																														\n"
"	gl_Position = projection * view * model * vec4(position, 1.0f);														\n"
"}";

constexpr const char *LIGHT_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"uniform mat4 model;																									\n"
"uniform mat4 view;																										\n"
"uniform mat4 projection;																								\n"

"in vec3 position;																										\n"
"in vec3 normal;																										\n"
"in vec3 tangent;																										\n"
"in vec2 uv;																											\n"

"out vec3 fragPos;																										\n"
"out vec2 textureUv;																									\n"
"out mat3 tbn;																											\n"

"void main()																											\n"
"{																														\n"
"	fragPos = (view * model * vec4(position, 1.0f)).xyz;																\n"
"	textureUv = uv;																										\n"

"	vec3 t = normalize((model * vec4(tangent, 0.0f)).xyz);																\n"
"	vec3 n = normalize((model * vec4(normal, 0.0f)).xyz);																\n"
"	vec3 b = cross(n, t);																								\n"
"	tbn = mat3(t, b, n);																								\n"

"	gl_Position = projection * view * model * vec4(position, 1.0f);														\n"
"}";

constexpr const char *DLIGHT_FRAG_SHDR_SRC =
"#version 430 core																										\n"

"uniform vec3 viewPos;																									\n"
"uniform vec3 lightDir;																									\n"
"uniform sampler2D bump;																								\n"
"uniform vec4 ambient;																									\n"
"uniform vec4 diffuse;																									\n"
"uniform vec4 specular;																									\n"

"in vec3 fragPos;																										\n"
"in vec2 textureUv;																										\n"
"in mat3 tbn;																											\n"

"out vec4 fragColor;																									\n"

"void main()																											\n"
"{																														\n"
"	vec3 viewDir = normalize(viewPos - fragPos);																		\n"
"	vec3 normal = tbn * normalize(texture(bump, textureUv).rgb * 2.0f - 1.0f);											\n"
"	float intensity = max(0.0f, dot(normal, lightDir));																	\n"

"	vec3 halfwayDir = normalize(lightDir + viewDir);																	\n"
"	float power = pow(max(dot(normal, halfwayDir), 0.0f), 1.0f);														\n"

"	fragColor = ambient + diffuse * intensity + specular * power;														\n"
"}";

constexpr const char *PLIGHT_FRAG_SHDR_SRC =
"#version 430 core																										\n"

"uniform vec3 viewPos;																									\n"
"uniform vec3 lightPos;																									\n"
"uniform sampler2D bump;																								\n"
"uniform vec3 attenuation;																								\n"
"uniform vec4 ambient;																									\n"
"uniform vec4 diffuse;																									\n"
"uniform vec4 specular;																									\n"

"in vec3 fragPos;																										\n"
"in vec2 textureUv;																										\n"
"in mat3 tbn;																											\n"

"out vec4 fragColor;																									\n"

"void main()																											\n"
"{																														\n"
"	vec3 viewDir = normalize(viewPos - fragPos);																		\n"
"	vec3 lightDir = normalize(lightPos - fragPos);																		\n"
"	vec3 normal = tbn * normalize(texture(bump, textureUv).rgb * 2.0f - 1.0f);											\n"
"	float intensity = max(0.0f, dot(normal, lightDir));																	\n"

"	float distance = length(lightPos - fragPos);																		\n"
"	float fa = 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * (distance * distance));				\n"
"	vec3 halfwayDir = normalize(lightDir + viewDir);																	\n"
"	float power = pow(max(dot(normal, halfwayDir), 0.0f), 1.0f);														\n"

"	fragColor = ambient * fa + diffuse * intensity * fa + specular * power * fa;										\n"
"}";

Plutonium::LightingRenderer::LightingRenderer(GraphicsAdapter * device)
	: device(device), state(0)
{
	/* Create required shaders. */
	shdrMesh = new Shader(MESH_VRTX_SHDR_SRC);
	shdrDLight = new Shader(LIGHT_VRTX_SHDR_SRC, DLIGHT_FRAG_SHDR_SRC);
	shdrPLight = new Shader(LIGHT_VRTX_SHDR_SRC, PLIGHT_FRAG_SHDR_SRC);

	/* Get mesh shader fields. */
	matMdlM = shdrMesh->GetUniform("model");
	matViewM = shdrMesh->GetUniform("view");
	matProjM = shdrMesh->GetUniform("projection");
	posM = shdrMesh->GetAttribute("position");

	/* Get directional light shader fields. */
	matMdlDl = shdrDLight->GetUniform("model");
	matViewDl = shdrDLight->GetUniform("view");
	matProjDl = shdrDLight->GetUniform("projection");
	camPosDl = shdrDLight->GetUniform("viewPos");
	mapBmpDl = shdrDLight->GetUniform("bump");
	ambiDl = shdrDLight->GetUniform("ambient");
	diffDl = shdrDLight->GetUniform("diffuse");
	specDl = shdrDLight->GetUniform("specular");
	dirDl = shdrDLight->GetUniform("lightDir");
	posDl = shdrDLight->GetAttribute("position");
	normDl = shdrDLight->GetAttribute("normal");
	tanDl = shdrDLight->GetAttribute("tangent");
	uvDl = shdrDLight->GetAttribute("uv");

	/* Get point light shader fields. */
	matMdlPl = shdrPLight->GetUniform("model");
	matViewPl = shdrPLight->GetUniform("view");
	matProjPl = shdrPLight->GetUniform("projection");
	camPosPl = shdrPLight->GetUniform("viewPos");
	mapBmpPl = shdrPLight->GetUniform("bump");
	ambiPl = shdrPLight->GetUniform("ambient");
	diffPl = shdrPLight->GetUniform("diffuse");
	specPl = shdrPLight->GetUniform("specular");
	lposPl = shdrPLight->GetUniform("lightPos");
	attenPl = shdrPLight->GetUniform("attenuation");
	posPl = shdrPLight->GetAttribute("position");
	normPl = shdrPLight->GetAttribute("normal");
	tanPl = shdrPLight->GetAttribute("tangent");
	uvPl = shdrPLight->GetAttribute("uv");
}

Plutonium::LightingRenderer::~LightingRenderer(void)
{
	delete_s(shdrMesh);
	delete_s(shdrDLight);
	delete_s(shdrPLight);
}

void Plutonium::LightingRenderer::BeginModels(const Matrix & view, const Matrix & proj)
{
	/* Check and update state. */
	ASSERT_IF(state != 0, "Cannot call BeginModels at this point!");
	state = 1;

	/* Initialize render pipeline. */
	shdrMesh->Begin();
	device->SetColorOutput(false, false, false, false);

	/* Save variables for later use. */
	this->view = view;
	this->proj = proj;

	/* Initialize global uniforms. */
	matViewM->Set(view);
	matProjM->Set(proj);
}

void Plutonium::LightingRenderer::BeginDirectionalLights(Vector3 camPos)
{
	/* Check and update state. */
	if (state == 1) shdrMesh->End();
	else if (state == 3) shdrPLight->End();
	else ASSERT("Cannot call BeginDirectionalLights at this point!");
	state = 2;

	/* Initialize render pipeline. */
	shdrDLight->Begin();
	device->SetDepthOuput(false);
	device->SetColorOutput(true, true, true, true);
	device->SetDepthTest(DepthState::Equal);
	device->SetColorBlendFunction(BlendState::Additive);

	/* Save variables for later use. */
	this->camPos = camPos;

	/* Initialize global uniforms. */
	matViewDl->Set(view);
	matProjDl->Set(proj);
	this->camPosDl->Set(camPos);
}

void Plutonium::LightingRenderer::BeginPointLights(void)
{
	/* Check and update state. */
	if (state == 1) shdrMesh->End();
	else if (state == 2) shdrDLight->End();
	else ASSERT("Cannot call BeginPointLights at this point!");
	state = 3;

	/* Initialize render pipeline. */
	shdrPLight->Begin();
	device->SetDepthOuput(false);
	device->SetColorOutput(true, true, true, true);
	device->SetDepthTest(DepthState::Equal);
	device->SetColorBlendFunction(BlendState::Additive);

	/* Initialize global uniforms. */
	matViewPl->Set(view);
	matProjPl->Set(proj);
	camPosPl->Set(camPos);
}

void Plutonium::LightingRenderer::Render(const Matrix & world, const Mesh * mesh)
{
	/* Set uniforms. */
	matMdlM->Set(world);

	/* Set attributes. */
	Buffer *buffer = mesh->GetVertexBuffer();
	buffer->Bind();
	posM->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render mesh. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
}

void Plutonium::LightingRenderer::Render(const Matrix & world, const Mesh * mesh, Texture * bumpMap, const DirectionalLight * light)
{
	/* Set uniforms. */
	matMdlDl->Set(world);
	mapBmpDl->Set(bumpMap);
	ambiDl->Set(light->Ambient);
	diffDl->Set(light->Diffuse);
	specDl->Set(light->Specular);
	dirDl->Set(light->Direction);

	/* Set attributes. */
	Buffer *buffer = mesh->GetVertexBuffer();
	buffer->Bind();
	posDl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	normDl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
	tanDl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
	uvDl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

	/* Render mesh. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
}

void Plutonium::LightingRenderer::Render(const Matrix & world, const Mesh * mesh, Texture * bumpMap, const PointLight * light)
{
	/* Set uniforms. */
	matMdlPl->Set(world);
	mapBmpPl->Set(bumpMap);
	ambiPl->Set(light->Ambient);
	diffPl->Set(light->Diffuse);
	specPl->Set(light->Specular);
	lposPl->Set(view *  light->Position);
	attenPl->Set(Vector3(light->Constant, light->Linear, light->Quadratic));

	/* Set attributes. */
	Buffer *buffer = mesh->GetVertexBuffer();
	buffer->Bind();
	posPl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	normPl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
	tanPl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
	uvPl->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

	/* Render mesh. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
}

void Plutonium::LightingRenderer::End(void)
{
	/* Check and update state. */
	if (state == 1) shdrMesh->End();
	else if (state == 2) shdrDLight->End();
	else if (state == 3) shdrPLight->End();
	else ASSERT("Cannot call End at this point!");
	state = 0;

	/* Reset render pipeline. */
	device->SetDepthOuput(true);
	device->SetDepthTest(DepthState::LessOrEqual);
	device->SetColorBlendFunction(BlendState::None);
}