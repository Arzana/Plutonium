#include "Graphics\Rendering\Deferred\DeferredRendererBP.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Materials\MaterialBP.h"

struct PlaneVertexFormat
{
	Plutonium::Vector3 Position;
	Plutonium::Vector2 Uv;
};

Plutonium::DeferredRendererBP::DeferredRendererBP(GraphicsAdapter * device)
	: device(device)
{
	/* Initialize g buffer. */
	fbo = new RenderTarget(device);
	normalSpec = fbo->Attach("Normal / Specular map", AttachmentOutputType::LpVector4);
	posSpec = fbo->Attach("Position / Specular exponent", AttachmentOutputType::LpVector4);
	ambient = fbo->Attach("Ambient", AttachmentOutputType::RGB);
	diffuse = fbo->Attach("Diffuse", AttachmentOutputType::RGB);
	fbo->Finalize();

	InitGPass();
	InitDPass();
	//InitPPass();
}

Plutonium::DeferredRendererBP::~DeferredRendererBP(void)
{
	delete_s(fbo);
	delete_s(gpass.shdr);
	delete_s(dpass.shdr);
	//delete_s(ppass.shdr);
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
	//queuePLights.push(light);
}

void Plutonium::DeferredRendererBP::Render(const Matrix & projection, const Matrix & view, Vector3 camPos)
{
	/* Perform geometry pass to fill the GBuffer. */
	BeginGPass(projection, view);
	while (queuedModels.size() > 0)
	{
		RenderModel(queuedModels.front());
		queuedModels.pop();
	}
	EndGPass();

	/* Add directional lights to the scene. */
	BeginDirLightPass(camPos);
	while (queuedDLights.size() > 0)
	{
		RenderDirLight(queuedDLights.front());
		queuedDLights.pop();
	}
	EndDirLightPass();

	fbo->BlitDepth();
}

void Plutonium::DeferredRendererBP::InitGPass(void)
{
	constexpr const char *VRTX_SHDR_SRC =
		"#version 430 core																			\n"

		"struct FragInfo																			\n"
		"{																							\n"
		"	vec3 Position;																			\n"
		"	vec2 Uv;																				\n"
		"	mat3 TBN;																				\n"
		"};																							\n"

		"uniform mat4 Projection;																	\n"
		"uniform mat4 View;																			\n"
		"uniform mat4 Model;																		\n"

		"in vec3 Position;																			\n"
		"in vec3 Normal;																			\n"
		"in vec3 Tangent;																			\n"
		"in vec2 Uv;																				\n"

		"out FragInfo Frag;																			\n"

		"void main()																				\n"
		"{																							\n"
		"	Frag.Position = (View * Model * vec4(Position, 1.0f)).xyz;								\n"
		"	Frag.Uv = Uv;																			\n"

		"	vec3 t = normalize((Model * vec4(Tangent, 0.0f)).xyz);									\n"
		"	vec3 n = normalize((Model * vec4(Normal, 0.0f)).xyz);									\n"
		"	vec3 b = cross(n, t);																	\n"
		"	Frag.TBN = mat3(t, b, n);																\n"

		"	gl_Position = Projection * View * Model * vec4(Position, 1.0f);							\n"
		"}";

	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																			\n"

		"struct FragInfo																			\n"
		"{																							\n"
		"	vec3 Position;																			\n"
		"	vec2 Uv;																				\n"
		"	mat3 TBN;																				\n"
		"};																							\n"

		"struct Material																			\n"
		"{																							\n"
		"	sampler2D Ambient;																		\n"
		"	sampler2D Diffuse;																		\n"
		"	sampler2D Specular;																		\n"
		"	sampler2D Opacity;																		\n"
		"	sampler2D Normal;																		\n"
		"	float SpecularExponent;																	\n"
		"};																							\n"

		"uniform Material Object;																	\n"
		"in FragInfo Frag;																			\n"

		"out vec4 NormalSpecular;																	\n"
		"out vec4 PositionSpecular;																	\n"
		"out vec3 Ambient;																			\n"
		"out vec3 Diffuse;																			\n"

		"void main()																				\n"
		"{																							\n"
		"	vec4 alpha = texture(Object.Opacity, Frag.Uv);											\n"
		"	if ((alpha.r < 0.1f && alpha.g < 0.1f && alpha.r < 0.1f) || alpha.a < 0.1f) discard;	\n"

		"	vec3 normal = texture(Object.Normal, Frag.Uv).rgb * 2.0f - 1.0f;						\n"
		"	normal = Frag.TBN * normalize(normal);													\n"

		"	Ambient = texture(Object.Ambient, Frag.Uv).rgb;											\n"
		"	Diffuse = texture(Object.Diffuse, Frag.Uv).rgb;											\n"
		"	NormalSpecular.xyz = normal;															\n"
		"	NormalSpecular.w = texture(Object.Specular, Frag.Uv).r;									\n"
		"	PositionSpecular.xyz = Frag.Position;													\n"
		"	PositionSpecular.w = Object.SpecularExponent;											\n"
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
		"#version 430 core																			\n"

		"in vec3 Position;																			\n"
		"in vec2 ScreenCoordinate;																	\n"

		"out vec2 Uv;																				\n"

		"void main()																				\n"
		"{																							\n"
		"	Uv = ScreenCoordinate;																	\n"
		"	gl_Position = vec4(Position, 1.0f);														\n"
		"}";

	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																			\n"

		"struct GBuffer																				\n"
		"{																							\n"
		"	sampler2D NormalSpecular;																\n"
		"	sampler2D PositionSpecular;																\n"
		"	sampler2D Ambient;																		\n"
		"	sampler2D Diffuse;																		\n"
		"};																							\n"

		"struct DirectionalLight																	\n"
		"{																							\n"
		"	vec3 Direction;																			\n"
		"	vec4 Ambient;																			\n"
		"	vec4 Diffuse;																			\n"
		"	vec4 Specular;																			\n"
		"};																							\n"

		"uniform GBuffer Geometry;																	\n"
		"uniform DirectionalLight Light;															\n"
		"uniform vec3 ViewPosition;																	\n"

		"in vec2 Uv;																				\n"

		"out vec4 FragColor;																		\n"

		"void main()																				\n"
		"{																							\n"
		"	vec4 posSpec = texture(Geometry.PositionSpecular, Uv);									\n"
		"	vec4 normSpec = texture(Geometry.NormalSpecular, Uv);									\n"
		"	vec3 objAmbient = texture(Geometry.Ambient, Uv).rgb;									\n"
		"	vec3 objDiffuse = texture(Geometry.Diffuse, Uv).rgb;									\n"

		"	vec3 viewDir = normalize(ViewPosition - posSpec.xyz);									\n"
		"	float intensity = max(0.0f, dot(normSpec.xyz, Light.Direction));						\n"
		"	vec3 halfwayDir = normalize(Light.Direction + viewDir);									\n"
		"	float power = pow(max(0.0f, dot(normSpec.xyz, halfwayDir)), posSpec.w);					\n"

		"	vec4 ambient = vec4(objAmbient, 1.0f) * Light.Ambient;									\n"
		"	vec4 diffuse = vec4(objDiffuse, 1.0f) * Light.Diffuse * intensity;						\n"
		"	vec4 specular = normSpec.w * Light.Specular * power;									\n"
		"	FragColor = ambient + diffuse + specular;												\n"
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

void Plutonium::DeferredRendererBP::BeginGPass(const Matrix & proj, const Matrix & view)
{
	/* Setup GBuffer. */
	device->SetRenderTarget(fbo);
	device->Clear(ClearTarget::Color | ClearTarget::Depth);

	/* Set geometry pass global parameters. */
	gpass.shdr->Begin();
	gpass.matProj->Set(proj);
	gpass.matview->Set(view);
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

void Plutonium::DeferredRendererBP::EndGPass(void)
{
	gpass.shdr->End();
	device->SetRenderTarget(nullptr);
}

void Plutonium::DeferredRendererBP::BeginDirLightPass(Vector3 camPos)
{
	/* Setup frame buffer and shader. */
	dpass.shdr->Begin();
	device->Clear(ClearTarget::Color);

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

void Plutonium::DeferredRendererBP::RenderDirLight(const DirectionalLight * light)
{
	dpass.dir->Set(light->Direction);
	dpass.clrAmbi->Set(light->Ambient);
	dpass.clrDiff->Set(light->Diffuse);
	dpass.clrSpec->Set(light->Specular);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(dpass.plane->GetElementCount()));
}

void Plutonium::DeferredRendererBP::EndDirLightPass(void)
{
	dpass.shdr->End();
}