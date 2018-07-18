#include "Graphics\Rendering\Deferred\DeferredRendererBP.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Materials\MaterialBP.h"
#include "Graphics\Models\Shapes.h"

//#define USE_LIGHT_VOLUME

/*
Deferred renderer works in three stages.

First stage:
- Accumilate all models that need to be rendered.
	- Static Models
- Perform a G Buffer pass for each of them (LDR).
	- Discard almost fully transparent fragments
	- Save material properies into the G buffer.

Second stage:
- Perform direction light pass for each directional light (HDR).
	- Apply to full screen quad.
	- Additively blend each light into HDR screen buffer.
- Perform point light pass for each point light (HDR).
	- Apply to full screen quad (later light volume (sphere) if I can get it working).
	- Additively blend each light into HDR screen buffer.

Third stage:
- Fix values for display monitor (LDR).
	- Reinhard tone mapping to go from HDR to LDR.
	- Apply gamma correction for each fragment.
*/

struct PlaneVertexFormat
{
	Plutonium::Vector3 Position;
	Plutonium::Vector2 Uv;
};

Plutonium::DeferredRendererBP::DeferredRendererBP(GraphicsAdapter * device)
	: device(device), Exposure(1.0f)
{
	/* Initialize G buffer. */
	gbFbo = new RenderTarget(device);
	normalSpec = gbFbo->Attach("Normal / Specular map", AttachmentOutputType::HpVector4);
	posSpec = gbFbo->Attach("Position / Specular exponent", AttachmentOutputType::HpVector4);
	ambient = gbFbo->Attach("Ambient", AttachmentOutputType::RGB);
	diffuse = gbFbo->Attach("Diffuse", AttachmentOutputType::RGB);
	gbFbo->Finalize();

	/* Initialize HDR lighting intermediate buffer. */
	hdrFbo = new RenderTarget(device);
	screen = hdrFbo->Attach("HDR Output", AttachmentOutputType::LpVector4);
	hdrFbo->Finalize();

	InitGPass();
	InitDPass();
	InitPPass();
	InitFPass();
}

Plutonium::DeferredRendererBP::~DeferredRendererBP(void)
{
	delete_s(gbFbo);
	delete_s(hdrFbo);
	delete_s(gpass.shdr);
	delete_s(dpass.shdr);
	delete_s(ppass.shdr);
}

void Plutonium::DeferredRendererBP::Add(const StaticObject * model)
{
	queuedModels.push(model);
}

void Plutonium::DeferredRendererBP::Add(const DirectionalLight * light)
{
	queuedDLights.push(light);
}

void Plutonium::DeferredRendererBP::Add(const PointLight * light)
{
	queuePLights.push(light);
}

void Plutonium::DeferredRendererBP::Render(const Matrix & projection, const Matrix & view, Vector3 camPos)
{
	/* Setup GBuffer. */
	device->SetRenderTarget(gbFbo);
	device->Clear(ClearTarget::Color | ClearTarget::Depth);

	/* Perform geometry pass to fill the GBuffer. */
	BeginGPass(projection, view);
	while (queuedModels.size() > 0)
	{
		RenderModel(queuedModels.front());
		queuedModels.pop();
	}
	gpass.shdr->End();

	/* Setup HDR buffer for lighting. */
	device->SetRenderTarget(hdrFbo);
	device->Clear(ClearTarget::Color | ClearTarget::Depth);
	device->SetColorBlendFunction(BlendState::Additive);

	/* Add directional lights to the scene. */
	BeginDirLightPass(camPos);
	const Matrix iView = view.GetOrientation().GetInverse();
	while (queuedDLights.size() > 0)
	{
		RenderDirLight(iView, queuedDLights.front());
		queuedDLights.pop();
	}
	dpass.shdr->End();

	/* Add point lights to the scene. */
	BeginPntLightPass(camPos);
	while (queuePLights.size() > 0)
	{
		RenderPntLight(queuePLights.front());
		queuePLights.pop();
	}
	ppass.shdr->End();

	/* Setup default frame buffer and copy the depth buffer over from the G buffer. */
	device->SetRenderTarget(nullptr);
	device->SetColorBlendFunction(BlendState::None);
	gbFbo->BlitDepth();

	/* Final render pass to fix the fix the values for the display monitor. */
	device->SetDepthOuput(false);
	FixForMonitor();
	device->SetDepthOuput(true);
}

void Plutonium::DeferredRendererBP::InitGPass(void)
{
	constexpr const char *VRTX_SHDR_SRC =
		"#version 430 core																										\n"

		"struct FragInfo																										\n"
		"{																														\n"
		"	vec3 Position;																										\n"
		"	vec2 Uv;																											\n"
		"	mat3 TBN;																											\n"
		"};																														\n"

		"uniform mat4 Projection;																								\n"
		"uniform mat4 View;																										\n"
		"uniform mat4 Model;																									\n"

		"in vec3 Position;																										\n"
		"in vec3 Normal;																										\n"
		"in vec3 Tangent;																										\n"
		"in vec2 Uv;																											\n"

		"out FragInfo Frag;																										\n"

		"void main()																											\n"
		"{																														\n"
		"	Frag.Position = (Model * vec4(Position, 1.0f)).xyz;																	\n"
		"	Frag.Uv = Uv;																										\n"

		"	vec3 t = normalize((Model * vec4(Tangent, 0.0f)).xyz);																\n"
		"	vec3 n = normalize((Model * vec4(Normal, 0.0f)).xyz);																\n"
		"	vec3 b = cross(n, t);																								\n"
		"	Frag.TBN = mat3(t, b, n);																							\n"

		"	gl_Position = Projection * View * Model * vec4(Position, 1.0f);														\n"
		"}";

	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																										\n"

		"struct FragInfo																										\n"
		"{																														\n"
		"	vec3 Position;																										\n"
		"	vec2 Uv;																											\n"
		"	mat3 TBN;																											\n"
		"};																														\n"

		"struct Material																										\n"
		"{																														\n"
		"	sampler2D Ambient;																									\n"
		"	sampler2D Diffuse;																									\n"
		"	sampler2D Specular;																									\n"
		"	sampler2D Opacity;																									\n"
		"	sampler2D Normal;																									\n"
		"	float SpecularExponent;																								\n"
		"};																														\n"

		"uniform Material Object;																								\n"
		"uniform float GammaCorrection;																							\n"
		"in FragInfo Frag;																										\n"

		"out vec4 NormalSpecular;																								\n"
		"out vec4 PositionSpecular;																								\n"
		"out vec3 Ambient;																										\n"
		"out vec3 Diffuse;																										\n"

		"void main()																											\n"
		"{																														\n"
		"	vec4 diffuse = texture(Object.Diffuse, Frag.Uv);																	\n"

		"	vec4 alpha = texture(Object.Opacity, Frag.Uv);																		\n"
		"	if ((alpha.r < 0.1f && alpha.g < 0.1f && alpha.r < 0.1f) || alpha.a < 0.1f || diffuse.a < 0.1f) discard;			\n"

		"	vec3 normal = texture(Object.Normal, Frag.Uv).rgb * 2.0f - 1.0f;													\n"
		"	normal = Frag.TBN * normalize(normal);																				\n"

		"	Ambient = texture(Object.Ambient, Frag.Uv).rgb;																		\n"
		"	Diffuse = diffuse.rgb;																								\n"
		"	NormalSpecular.xyz = normal;																						\n"
		"	NormalSpecular.w = texture(Object.Specular, Frag.Uv).r;																\n"
		"	PositionSpecular.xyz = Frag.Position;																				\n"
		"	PositionSpecular.w = Object.SpecularExponent;																		\n"
		"}";

	gpass.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	gpass.matProj = gpass.shdr->GetUniform("Projection");
	gpass.matview = gpass.shdr->GetUniform("View");
	gpass.matMdl = gpass.shdr->GetUniform("Model");
	gpass.specExp = gpass.shdr->GetUniform("Object.SpecularExponent");
	gpass.mapAmbi = gpass.shdr->GetUniform("Object.Ambient");
	gpass.mapDiff = gpass.shdr->GetUniform("Object.Diffuse");
	gpass.mapSpec = gpass.shdr->GetUniform("Object.Specular");
	gpass.mapAlpha = gpass.shdr->GetUniform("Object.Opacity");
	gpass.mapBump = gpass.shdr->GetUniform("Object.Normal");
	gpass.pos = gpass.shdr->GetAttribute("Position");
	gpass.norm = gpass.shdr->GetAttribute("Normal");
	gpass.tan = gpass.shdr->GetAttribute("Tangent");
	gpass.uv = gpass.shdr->GetAttribute("Uv");
}

void Plutonium::DeferredRendererBP::InitDPass(void)
{
	constexpr const char *VRTX_SHDR_SRC =
		"#version 430 core																										\n"

		"in vec3 Position;																										\n"
		"in vec2 ScreenCoordinate;																								\n"

		"out vec2 Uv;																											\n"

		"void main()																											\n"
		"{																														\n"
		"	Uv = ScreenCoordinate;																								\n"
		"	gl_Position = vec4(Position, 1.0f);																					\n"
		"}";

	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																										\n"

		"struct GBuffer																											\n"
		"{																														\n"
		"	sampler2D NormalSpecular;																							\n"
		"	sampler2D PositionSpecular;																							\n"
		"	sampler2D Ambient;																									\n"
		"	sampler2D Diffuse;																									\n"
		"};																														\n"

		"struct DirectionalLight																								\n"
		"{																														\n"
		"	vec3 Direction;																										\n"
		"	vec4 Ambient;																										\n"
		"	vec4 Diffuse;																										\n"
		"	vec4 Specular;																										\n"
		"};																														\n"

		"uniform GBuffer Geometry;																								\n"
		"uniform DirectionalLight Light;																						\n"
		"uniform vec3 ViewPosition;																								\n"

		"in vec2 Uv;																											\n"

		"out vec4 FragColor;																									\n"

		"void main()																											\n"
		"{																														\n"
		"	vec4 posSpec = texture(Geometry.PositionSpecular, Uv);																\n"
		"	vec4 normSpec = texture(Geometry.NormalSpecular, Uv);																\n"
		"	vec3 objAmbient = texture(Geometry.Ambient, Uv).rgb;																\n"
		"	vec3 objDiffuse = texture(Geometry.Diffuse, Uv).rgb;																\n"

		"	vec3 viewDir = normalize(ViewPosition - posSpec.xyz);																\n"
		"	float intensity = max(0.0f, dot(normSpec.xyz, Light.Direction));													\n"
		"	vec3 halfwayDir = normalize(Light.Direction + viewDir);																\n"
		"	float power = pow(max(0.0f, dot(normSpec.xyz, halfwayDir)), posSpec.w);												\n"

		"	vec4 ambient = vec4(objAmbient, 1.0f) * Light.Ambient;																\n"
		"	vec4 diffuse = vec4(objDiffuse, 1.0f) * Light.Diffuse * intensity;													\n"
		"	vec4 specular = normSpec.w * Light.Specular * power;																\n"
		"	FragColor = ambient + diffuse + specular;																			\n"
		"}";

	dpass.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	dpass.normSpec = dpass.shdr->GetUniform("Geometry.NormalSpecular");
	dpass.posSpec = dpass.shdr->GetUniform("Geometry.PositionSpecular");
	dpass.ambi = dpass.shdr->GetUniform("Geometry.Ambient");
	dpass.diff = dpass.shdr->GetUniform("Geometry.Diffuse");
	dpass.dir = dpass.shdr->GetUniform("Light.Direction");
	dpass.clrAmbi = dpass.shdr->GetUniform("Light.Ambient");
	dpass.clrDiff = dpass.shdr->GetUniform("Light.Diffuse");
	dpass.clrSpec = dpass.shdr->GetUniform("Light.Specular");
	dpass.camPos = dpass.shdr->GetUniform("ViewPosition");
	dpass.pos = dpass.shdr->GetAttribute("Position");
	dpass.uv = dpass.shdr->GetAttribute("ScreenCoordinate");

	/* Initialize plane used for fully screen rendering. */
	dpass.plane = new Buffer(device->GetWindow(), BindTarget::Array);
	PlaneVertexFormat vertices[4] =
	{
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f) }
	};
	dpass.plane->SetData(BufferUsage::StaticDraw, vertices, 4);
}

void Plutonium::DeferredRendererBP::InitPPass(void)
{
	constexpr const char *VRTX_SHDR_SRC =
		"#version 430 core																										\n"
		
		"uniform mat4 Model;																									\n"
		
		"in vec3 Position;																										\n"
		"in vec2 ScreenCoordinate;																								\n"
		
		"out vec2 Uv;																											\n"
		
		"void main()																											\n"
		"{																														\n"
		"	Uv = ScreenCoordinate;																								\n"
		"	gl_Position = Model * vec4(Position, 1.0f);																			\n"
		"}";

	constexpr const char *FRAG_SHDR_SRC = 
		"#version 430 core																										\n"
		
		"struct GBuffer																											\n"
		"{																														\n"
		"	sampler2D NormalSpecular;																							\n"
		"	sampler2D PositionSpecular;																							\n"
		"	sampler2D Ambient;																									\n"
		"	sampler2D Diffuse;																									\n"
		"};																														\n"
		
		"struct PointLight																										\n"
		"{																														\n"
		"	vec3 Position;																										\n"
		"	float Constant;																										\n"
		"	float Linear;																										\n"
		"	float Quadratic;																									\n"
		"	vec4 Ambient;																										\n"
		"	vec4 Diffuse;																										\n"
		"	vec4 Specular;																										\n"
		"};																														\n"
		
		"uniform GBuffer Geometry;																								\n"
		"uniform PointLight Light;																								\n"
		"uniform vec3 ViewPosition;																								\n"

		"in vec2 Uv;																											\n"

		"out vec4 FragColor;																									\n"
		
		"void main()																											\n"
		"{																														\n"
		"	vec4 posSpec = texture(Geometry.PositionSpecular, Uv);																\n"
		"	vec4 normSpec = texture(Geometry.NormalSpecular, Uv);																\n"
		"	vec3 objAmbient = texture(Geometry.Ambient, Uv).rgb;																\n"
		"	vec3 objDiffuse = texture(Geometry.Diffuse, Uv).rgb;																\n"

		"	vec3 viewDir = normalize(ViewPosition - posSpec.xyz);																\n"
		"	vec3 lightDir = normalize(Light.Position - posSpec.xyz);															\n"
		"	float distance = length(Light.Position - posSpec.xyz);																\n"

		"	float intensity = max(0.0f, dot(normSpec.xyz, lightDir));															\n"
		"	float attenuation = 1.0f / (Light.Constant + Light.Linear * distance + Light.Quadratic * distance * distance);		\n"
		"	vec3 halfwayDir = normalize(lightDir + viewDir);																	\n"
		"	float power = pow(max(0.0f, dot(normSpec.xyz, halfwayDir)), posSpec.w);												\n"

		"	vec4 ambient = vec4(objAmbient, 1.0f) * Light.Ambient * attenuation;												\n"
		"	vec4 diffuse = vec4(objDiffuse, 1.0f) * Light.Diffuse * attenuation * intensity;									\n"
		"	vec4 specular = normSpec.w * Light.Specular * attenuation * power;													\n"
		"	FragColor = ambient + diffuse + specular;																			\n"
		"}";

	ppass.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	ppass.normSpec = ppass.shdr->GetUniform("Geometry.NormalSpecular");
	ppass.posSpec = ppass.shdr->GetUniform("Geometry.PositionSpecular");
	ppass.ambi = ppass.shdr->GetUniform("Geometry.Ambient");
	ppass.diff = ppass.shdr->GetUniform("Geometry.Diffuse");
	ppass.lpos = ppass.shdr->GetUniform("Light.Position");
	ppass.c = ppass.shdr->GetUniform("Light.Constant");
	ppass.l = ppass.shdr->GetUniform("Light.Linear");
	ppass.q = ppass.shdr->GetUniform("Light.Quadratic");
	ppass.clrAmbi = ppass.shdr->GetUniform("Light.Ambient");
	ppass.clrDiff = ppass.shdr->GetUniform("Light.Diffuse");
	ppass.clrSpec = ppass.shdr->GetUniform("Light.Specular");
	ppass.camPos = ppass.shdr->GetUniform("ViewPosition");
	ppass.matMdl = ppass.shdr->GetUniform("Model");
	ppass.pos = ppass.shdr->GetAttribute("Position");
	ppass.uv = ppass.shdr->GetAttribute("ScreenCoordinate");

	/* Initialize sphere used for point light rendering. */
	ppass.sphere = new Mesh("Point light render spherer");
	ShapeCreator::MakeSphere(ppass.sphere, 16, 16, 1.0f);
	ppass.sphere->Finalize(device->GetWindow());
}

void Plutonium::DeferredRendererBP::InitFPass(void)
{
	constexpr const char *VRTX_SHDR_SRC =
		"#version 430 core																										\n"

		"in vec3 Position;																										\n"
		"in vec2 ScreenCoordinate;																								\n"

		"out vec2 Uv;																											\n"

		"void main()																											\n"
		"{																														\n"
		"	Uv = ScreenCoordinate;																								\n"
		"	gl_Position = vec4(Position, 1.0f);																					\n"
		"}";

	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																										\n"
		
		"uniform sampler2D HdrBuffer;																							\n"
		"uniform float GammaCorrection;																							\n"
		"uniform float Exposure;																								\n"
		
		"in vec2 Uv;																											\n"

		"out vec4 FragColor;																									\n"
		
		"void main()																											\n"
		"{																														\n"
		"	vec3 hdr = texture(HdrBuffer, Uv).rgb;																				\n"
		"	vec3 mapped = vec3(1.0f) - exp(-hdr * Exposure);																	\n"
		"	vec3 corrected = pow(mapped, vec3(GammaCorrection));																\n"
		"	FragColor = vec4(corrected, 1.0f);																					\n"
		"}";

	fpass.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	fpass.screen = fpass.shdr->GetUniform("HdrBuffer");
	fpass.gamma = fpass.shdr->GetUniform("GammaCorrection");
	fpass.exposure = fpass.shdr->GetUniform("Exposure");
	fpass.pos = fpass.shdr->GetAttribute("Position");
	fpass.uv = fpass.shdr->GetAttribute("ScreenCoordinate");
}

void Plutonium::DeferredRendererBP::BeginGPass(const Matrix & proj, const Matrix & view)
{
	/* Set geometry pass global parameters. */
	gpass.shdr->Begin();
	gpass.matProj->Set(proj);
	gpass.matview->Set(view);
}

void Plutonium::DeferredRendererBP::BeginDirLightPass(Vector3 camPos)
{
	/* Setup frame buffer and shader. */
	dpass.shdr->Begin();

	/* Setup global uniforms. */
	dpass.camPos->Set(camPos);
	dpass.normSpec->Set(normalSpec);
	dpass.posSpec->Set(posSpec);
	dpass.ambi->Set(ambient);
	dpass.diff->Set(diffuse);

	/* Setup mesh. */
	dpass.plane->Bind();
	dpass.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	dpass.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));
}

void Plutonium::DeferredRendererBP::BeginPntLightPass(Vector3 camPos)
{
	ppass.shdr->Begin();

	/* Setup global uniforms. */
	ppass.camPos->Set(camPos);
	ppass.normSpec->Set(normalSpec);
	ppass.posSpec->Set(posSpec);
	ppass.ambi->Set(ambient);
	ppass.diff->Set(diffuse);

	/* Setup mesh. */
#if defined (USE_LIGHT_VOLUME)
	ppass.sphere->GetVertexBuffer()->Bind();
	ppass.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	ppass.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));
#else
	dpass.plane->Bind();
	ppass.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	ppass.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));
#endif
}

void Plutonium::DeferredRendererBP::RenderModel(const StaticObject * model)
{
	/* Set model matrix. */
	gpass.matMdl->Set(model->GetWorld());

	/* Loops through all shapes in the model. */
	const std::vector<StaticModel::Shape> *shapes = model->GetModel()->GetShapes();
	for (size_t i = 0; i < shapes->size(); i++)
	{
		/* Render the current material if it's visible. */
		MaterialBP *material = shapes->at(i).Material;
		if (material->Visible)
		{
			Buffer *mesh = shapes->at(i).Mesh->GetVertexBuffer();

			/* Set material attributes. */
			gpass.mapAmbi->Set(material->Ambient);
			gpass.mapDiff->Set(material->Diffuse);
			gpass.mapSpec->Set(material->Specular);
			gpass.mapAlpha->Set(material->Opacity);
			gpass.mapBump->Set(material->Normal);
			gpass.specExp->Set(material->SpecularExponent);

			/* Set mesh attributes. */
			mesh->Bind();
			gpass.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
			gpass.norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
			gpass.tan->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
			gpass.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

			/* Render the shape to the GBuffer. */
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh->GetElementCount()));
		}
	}
}

void Plutonium::DeferredRendererBP::RenderDirLight(const Matrix &iview, const DirectionalLight * light)
{
	dpass.dir->Set(normalize(light->Direction));
	dpass.clrAmbi->Set(light->Ambient);
	dpass.clrDiff->Set(light->Diffuse);
	dpass.clrSpec->Set(light->Specular);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(dpass.plane->GetElementCount()));
}

void Plutonium::DeferredRendererBP::RenderPntLight(const PointLight * light)
{
#if defined (USE_LIGHT_VOLUME)
	ppass.matMdl->Set(Matrix::CreateTranslation(light->Position) * Matrix::CreateScalar(light->GetRadius()));
#else
	ppass.matMdl->Set(Matrix());
#endif
	ppass.lpos->Set(light->Position);
	ppass.c->Set(light->Constant);
	ppass.l->Set(light->Linear);
	ppass.q->Set(light->Quadratic);
	ppass.clrAmbi->Set(light->Ambient);
	ppass.clrDiff->Set(light->Diffuse);
	ppass.clrSpec->Set(light->Specular);

#if defined (USE_LIGHT_VOLUME)
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(ppass.sphere->GetVertexBuffer()->GetElementCount()));
#else
	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(dpass.plane->GetElementCount()));
#endif
}

void Plutonium::DeferredRendererBP::FixForMonitor(void)
{
	/* Start shader. */
	fpass.shdr->Begin();
	
	/* Set uniforms. */
	fpass.screen->Set(screen);
	fpass.gamma->Set(device->GetWindow()->GetGraphicsDevice().GammaCorrection);
	fpass.exposure->Set(Exposure);

	/* Setup screen quad (use dpass quad plane). */
	dpass.plane->Bind();
	fpass.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	fpass.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));

	/* Fix for monitor for each fragment and end shader. */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(dpass.plane->GetElementCount()));
	fpass.shdr->End();
}