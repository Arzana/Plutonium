#include "Graphics\Rendering\Deferred\DeferredRendererBP.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Materials\MaterialBP.h"
#include "Graphics\Models\Shapes.h"
#include "Core\Math\Interpolation.h"

#define PCF

/*
Deferred renderer works in three stages.

First stage:
- Accumilate all models that need to be rendered.
	- Static Models
	- Dynamic Models
- Perform a G Buffer pass for each of them (LDR).
	- Discard almost fully transparent fragments
	- Save material properies into the G buffer.

Second stage:
- Perform direction light pass for each directional light (HDR).
	- Render shadow maps (PSSM / CSM).
	- Apply to full screen quad.
	- PCF smooth shadows.
	- Additively blend each light into HDR screen buffer.
- Perform point light pass for each point light (HDR).
	- Apply to light volume sphere.
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

#pragma region Shader Sources
constexpr const char *STATIC_GPASS_VRTX_SHDR_SRC =
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

constexpr const char *ANIMATED_GPASS_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"struct FragInfo																										\n"
"{																														\n"
"	vec3 Position;																										\n"
"	vec2 Uv;																											\n"
"	mat3 TBN;																											\n"
"};																														\n"

"uniform mat4 Model;																									\n"
"uniform mat4 View;																										\n"
"uniform mat4 Projection;																								\n"
"uniform float Blending;																								\n"

"in vec3 Position1;																										\n"
"in vec3 Normal1;																										\n"
"in vec3 Tangent1;																										\n"
"in vec2 Uv;																											\n"

"in vec3 Position2;																										\n"
"in vec3 Normal2;																										\n"
"in vec3 Tangent2;																										\n"

"out FragInfo Frag;																										\n"

"void main()																											\n"
"{																														\n"
"	vec3 position = mix(Position1, Position2, Blending);																\n"
"	vec3 normal = normalize(Normal1 + Normal2);																			\n"
"	vec3 tangent = normalize(Tangent1 + Tangent2);																		\n"

"	Frag.Position = (Model * vec4(position, 1.0f)).xyz;																	\n"
"	Frag.Uv = Uv;																										\n"

"	vec3 t = normalize((Model * vec4(tangent, 0.0f)).xyz);																\n"
"	vec3 n = normalize((Model * vec4(normal, 0.0f)).xyz);																\n"
"	vec3 b = cross(n, t);																								\n"
"	Frag.TBN = mat3(t, b, n);																							\n"

"	gl_Position = Projection * View * Model * vec4(position, 1.0f);														\n"
"}";

constexpr const char *GPASS_FRAG_SHDR_SRC =
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

constexpr const char *PASS_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"in vec3 Position;																										\n"
"in vec2 ScreenCoordinate;																								\n"

"out vec2 Uv;																											\n"

"void main()																											\n"
"{																														\n"
"	Uv = ScreenCoordinate;																								\n"
"	gl_Position = vec4(Position, 1.0f);																					\n"
"}";

constexpr const char *DSPASS_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"uniform mat4 Model;																									\n"
"uniform mat4 CascadeSpace;																								\n"
"uniform float Blending;																								\n"

"in vec3 Position1;																										\n"
"in vec3 Position2;																										\n"
"in vec2 Uv;																											\n"

"out vec2 FragUv;																										\n"

"void main()																											\n"
"{																														\n"
"	FragUv = Uv;																										\n"
"	gl_Position = CascadeSpace * Model * vec4(mix(Position1, Position2, Blending), 1.0f);								\n"
"}";

constexpr const char *DPASS_FRAG_SHDR_SRC =
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
"	mat4 CascadeSpace1;																									\n"
"	sampler2D Cascade1;																									\n"
"	float CascadeEnd1;																									\n"
"	mat4 CascadeSpace2;																									\n"
"	sampler2D Cascade2;																									\n"
"	float CascadeEnd2;																									\n"
"	mat4 CascadeSpace3;																									\n"
"	sampler2D Cascade3;																									\n"
"	float CascadeEnd3;																									\n"
"};																														\n"

"uniform GBuffer Geometry;																								\n"
"uniform DirectionalLight Light;																						\n"
"uniform vec3 ViewPosition;																								\n"
"uniform mat4 View;																										\n"
"uniform float CascadeIntensity;																						\n"

"in vec2 Uv;																											\n"

"out vec4 FragColor;																									\n"

"float CalcShadowFactor(sampler2D cascade, vec4 pos, vec3 normal)														\n"
"{																														\n"
"	vec3 ndc = (pos.xyz / pos.w) * 0.5f + 0.5f;																			\n"
"	float bias = max(0.005f * (1.0f - dot(normal, Light.Direction)), 0.005f);											\n"

#if defined (PCF)
"	float result = 0.0f;																								\n"
"	vec2 texelSize = 1.0f / textureSize(cascade, 0);																	\n"
"	for (float x = -1.5f; x <= 1.5f; x += 1.0f)																			\n"
"	{																													\n"
"		for (float y = -1.5f; y <= 1.5f; y += 1.0f)																		\n"
"		{																												\n"
"			float pcf = texture(cascade, ndc.xy + vec2(x, y) * texelSize).x;											\n"
"			result += pcf < ndc.z - bias ? 0.0f : 1.0f;																	\n"
"		}																												\n"
"	}																													\n"

"	return result / 16.0f;																								\n"
#else
"	return texture(cascade, ndc.xy).x < ndc.z - bias ? 0.0f : 1.0f;														\n"
#endif
"}																														\n"

"void main()																											\n"
"{																														\n"
"	vec4 posSpec = texture(Geometry.PositionSpecular, Uv);																\n"
"	vec4 normSpec = texture(Geometry.NormalSpecular, Uv);																\n"
"	vec3 objAmbient = texture(Geometry.Ambient, Uv).rgb;																\n"
"	vec3 objDiffuse = texture(Geometry.Diffuse, Uv).rgb;																\n"

"	vec4 pos = vec4(posSpec.xyz, 1.0f);																					\n"
"	float viewSpaceZ = -(View * pos).z;																					\n"
"	float visibility = 0.0f;																							\n"
"	if (viewSpaceZ <= Light.CascadeEnd1)																				\n"
"	{																													\n"
"		visibility = CalcShadowFactor(Light.Cascade1, Light.CascadeSpace1 * pos, normSpec.xyz);							\n"
"		objAmbient.r = max(CascadeIntensity, objAmbient.r);																\n"
"	}																													\n"
"	else if (viewSpaceZ <= Light.CascadeEnd2)																			\n"
"	{																													\n"
"		visibility = CalcShadowFactor(Light.Cascade2, Light.CascadeSpace2 * pos, normSpec.xyz);							\n"
"		objAmbient.g = max(CascadeIntensity, objAmbient.g);																\n"
"	}																													\n"
"	else if (viewSpaceZ <= Light.CascadeEnd3)																			\n"
"	{																													\n"
"		visibility = CalcShadowFactor(Light.Cascade3, Light.CascadeSpace3 * pos, normSpec.xyz);							\n"
"		objAmbient.b = max(CascadeIntensity, objAmbient.b);																\n"
"	}																													\n"

"	vec3 viewDir = normalize(ViewPosition - posSpec.xyz);																\n"
"	float intensity = max(0.0f, dot(normSpec.xyz, Light.Direction));													\n"
"	vec3 halfwayDir = normalize(Light.Direction + viewDir);																\n"
"	float power = pow(max(0.0f, dot(normSpec.xyz, halfwayDir)), posSpec.w);												\n"

"	vec4 ambient = vec4(objAmbient, 1.0f) * Light.Ambient;																\n"
"	vec4 diffuse = vec4(objDiffuse, 1.0f) * Light.Diffuse * intensity;													\n"
"	vec4 specular = normSpec.w * Light.Specular * power;																\n"
"	FragColor = ambient + (visibility * (diffuse + specular));															\n"
"}";


constexpr const char* DSPASS_FRAG_SHDR_SRC =
"#version 430 core																										\n"

"uniform sampler2D Opacity;																								\n"
"uniform sampler2D Diffuse;																								\n"

"in vec2 FragUv;																										\n"

"void main()																											\n"
"{																														\n"
"	vec4 alpha = texture(Opacity, FragUv);																				\n"
"	float diffuse = texture(Diffuse, FragUv).a;																			\n"
"	if ((alpha.r < 0.1f && alpha.g < 0.1f && alpha.r < 0.1f) || alpha.a < 0.1f || diffuse < 0.1f) discard;				\n"
"	gl_FragDepth = gl_FragCoord.z;																						\n"
"}";

constexpr const char *PPASS_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"uniform mat4 Projection;																								\n"
"uniform mat4 View;																										\n"
"uniform mat4 Model;																									\n"

"in vec3 Position;																										\n"

"void main()																											\n"
"{																														\n"
"	gl_Position = Projection * View * Model * vec4(Position, 1.0f);														\n"
"}";

constexpr const char *PPASS_FRAG_SHDR_SRC =
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
"uniform vec2 ScreenSize;																								\n"

"out vec4 FragColor;																									\n"

"void main()																											\n"
"{																														\n"
"	vec2 uv = gl_FragCoord.xy / ScreenSize;																				\n"
"	vec4 posSpec = texture(Geometry.PositionSpecular, uv);																\n"
"	vec4 normSpec = texture(Geometry.NormalSpecular, uv);																\n"
"	vec3 objAmbient = texture(Geometry.Ambient, uv).rgb;																\n"
"	vec3 objDiffuse = texture(Geometry.Diffuse, uv).rgb;																\n"

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
"	FragColor = ambient + diffuse + specular, vec4(1.0f);																\n"
"}";

constexpr const char *FPASS_FRAG_SHDR_SRC =
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

constexpr const char *WIRE_VRTX_SHDR_SRC =
"#version 430 core																										\n"

"uniform mat4 Model;																									\n"
"uniform mat4 View;																										\n"
"uniform mat4 Projection;																								\n"
"uniform float Blending;																								\n"

"in vec3 Position;																										\n"
"in vec3 Position2;																										\n"
"in vec2 Uv;																											\n"

"out vec2 GeomUv;																										\n"

"void main()																											\n"
"{																														\n"
"	GeomUv = Uv;																										\n"
"	gl_Position = Projection * View * Model * vec4(mix(Position, Position2, Blending), 1.0);							\n"
"}";

constexpr const char *WIRE_GEOM_SHDR_SRC =
"#version 430 core																										\n"

"layout (triangles) in;																									\n"
"layout (triangle_strip, max_vertices = 3) out;																			\n"

"in vec2 GeomUv[];																										\n"

"out vec2 FragUv;																										\n"
"out noperspective vec3 WireFrameDist;																					\n"

"void main()																											\n"
"{																														\n"
"	for (int i = 0; i < 3; i++)																							\n"
"	{																													\n"
"		gl_Position = gl_in[i].gl_Position;																				\n"
"		FragUv = GeomUv[i];																									\n"
"		WireFrameDist = vec3(0.0f);																						\n"
"		WireFrameDist[i] = 1.0f;																						\n"

"		EmitVertex();																									\n"
"	}																													\n"
"}";

constexpr const char *WIRE_FRAG_SHDR_SRC =
"#version 430 core																										\n"

"uniform vec4 Color;																									\n"
"uniform sampler2D Diffuse;																								\n"

"in vec2 FragUv;																										\n"
"in vec3 WireFrameDist;																									\n"

"out vec4 FragColor;																									\n"

"void main()																											\n"
"{																														\n"
"	vec3 d = fwidth(WireFrameDist);																						\n"
"	vec3 a = smoothstep(vec3(0.0f), d * 1.5f, WireFrameDist);															\n"
"	float edgeFactor = min(min(a.x, a.y), a.z);																			\n"

"	vec4 base = vec4(texture(Diffuse, FragUv).rgb, 0.5f);																\n"
"	FragColor = mix(Color, base, edgeFactor);																			\n"
"}";

constexpr const char *WN_FRAG_SHDR_SRC =
"#version 430 core																										\n"

"uniform sampler2D NormalSpecular;																						\n"

"in vec2 Uv;																											\n"

"out vec4 FragColor;																									\n"

"void main()																											\n"
"{																														\n"
"	vec3 worldNormal = texture(NormalSpecular, Uv).xyz;																	\n"
"	vec3 rgbNormal = normalize(worldNormal * 0.5f + 0.5f);																\n"
"	FragColor = vec4(rgbNormal, 1.0f);																					\n"
"}";

constexpr const char *AL_FRAG_SHDR_SRC =
"#version 430 core																										\n"

"uniform sampler2D Diffuse;																								\n"

"in vec2 Uv;																											\n"

"out vec4 FragColor;																									\n"

"void main()																											\n"
"{																														\n"
"	FragColor = vec4(texture(Diffuse, Uv).rgb, 1.0f);																	\n"
"}";

#pragma endregion

Plutonium::DeferredRendererBP::DeferredRendererBP(Game * game)
	: game(game), Exposure(1.0f), CascadeLambda(0.5f), LightOffset(1.0f), DisplayType(RenderType::Normal)
{
	/* Initialize G buffer. */
	gbFbo = new RenderTarget(game->GetGraphics(), true);
	normalSpec = gbFbo->Attach("Normal / Specular map", AttachmentOutputType::HpVector4);
	posSpec = gbFbo->Attach("Position / Specular exponent", AttachmentOutputType::HpVector4);
	ambient = gbFbo->Attach("Ambient", AttachmentOutputType::RGB);
	diffuse = gbFbo->Attach("Diffuse", AttachmentOutputType::RGB);
	gbFbo->Finalize();

	/* Initialize shader map buffer. */
	dshdFbo = new RenderTarget(game->GetGraphics(), false, 1024, 4096);
	for (size_t i = 0; i < CASCADE_CNT; i++) cascades[i] = dshdFbo->Attach("Directional Light Cascade Depth", AttachmentOutputType::HpDepth, i == 0);
	dshdFbo->Finalize();

	/* Initialize HDR lighting intermediate buffer. */
	hdrFbo = new RenderTarget(game->GetGraphics(), true);
	screen = hdrFbo->Attach("HDR Output", AttachmentOutputType::LpVector4);
	hdrFbo->Finalize();

	InitMeshes();
	InitGsPass();
	InitGdPass();
	InitDsPass();
	InitDPass();
	InitPPass();
	InitFPass();
	InitWire();
	InitWNormal();
	InitAlbedo();
}

Plutonium::DeferredRendererBP::~DeferredRendererBP(void)
{
	delete_s(gbFbo);
	delete_s(dshdFbo);
	delete_s(hdrFbo);
	delete_s(defMaterial);
	delete_s(plane);
	delete_s(sphere);
	delete_s(gspass.shdr);
	delete_s(gdpass.shdr);
	delete_s(dspass.shdr);
	delete_s(dpass.shdr);
	delete_s(ppass.shdr);
	delete_s(wire.shdr);
	delete_s(wn.shdr);
	delete_s(al.shdr);
}

void Plutonium::DeferredRendererBP::Add(const StaticObject * model)
{
	queuedModels.push_back(model);
}

void Plutonium::DeferredRendererBP::Add(const DynamicObject * model)
{
	queuedAnimations.push_back(model);
}

void Plutonium::DeferredRendererBP::Add(const DirectionalLight * light)
{
	queuedDLights.push_back(light);
}

void Plutonium::DeferredRendererBP::Add(const PointLight * light)
{
	queuePLights.push_back(light);
}

void Plutonium::DeferredRendererBP::Render(const Camera * cam)
{
	GraphicsAdapter *device = game->GetGraphics();
	const MaterialBP *usedMaterial = DisplayType != RenderType::Lighting ? nullptr : defMaterial;

	/* Setup GBuffer. */
	device->SetRenderTarget(gbFbo);
	device->Clear(ClearTarget::Color | ClearTarget::Depth);
	device->SetColorBlendFunction(BlendState::None);

	/* Perform geometry pass to fill the GBuffer. */
	BeginGsPass(cam->GetProjection(), cam->GetView());
	for (size_t i = 0; i < queuedModels.size(); i++) RenderModel(cam, queuedModels.at(i), usedMaterial);
	gspass.shdr->End();

	BeginGdPass(cam->GetProjection(), cam->GetView());
	for (size_t i = 0; i < queuedAnimations.size(); i++) RenderModel(cam, queuedAnimations.at(i), usedMaterial);
	gdpass.shdr->End();

	/* Setup HDR buffer for lighting. */
	device->SetRenderTarget(hdrFbo);
	device->Clear(ClearTarget::Color | ClearTarget::Depth);

	/* Render to HDR buffer is defined per render mode. */
	switch (DisplayType)
	{
	case Plutonium::RenderType::Normal:
	case Plutonium::RenderType::Lighting:
	case Plutonium::RenderType::Shadows:
		RenderNormal(cam);
		break;
	case Plutonium::RenderType::Wireframe:
		RenderWireframe(cam->GetProjection(), cam->GetView());
		break;
	case Plutonium::RenderType::WorldNormals:
		RenderWorldNormals();
		break;
	case Plutonium::RenderType::Albedo:
		RenderAlbedo();
		break;
	default:
		LOG_THROW("Display type is not supported!");
		break;
	}

	/* Setup default frame buffer and copy the depth buffer over from the G buffer (or hdr buffer for wireframe mode). */
	device->SetRenderTarget(nullptr);
	device->SetColorBlendFunction(BlendState::None);
	device->SetDepthOuput(true);
	gbFbo->BlitDepth();

	/* Final render pass to fix the fix the values for the display monitor. */
	device->SetDepthOuput(false);
	FixForMonitor();
	device->SetDepthOuput(true);

	/* Clear queues. */
	queuedModels.clear();
	queuedAnimations.clear();
	queuedDLights.clear();
	queuePLights.clear();
}

#pragma region Initializers
void Plutonium::DeferredRendererBP::InitMeshes(void)
{
	/* Initialize plane used for fully screen rendering. */
	plane = new Buffer(game->GetGraphics()->GetWindow(), BindTarget::Array);
	PlaneVertexFormat vertices[4] =
	{
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f) }
	};
	plane->SetData(BufferUsage::StaticDraw, vertices, 4);

	/* Initialize sphere used for point light rendering. */
	sphere = new Mesh("Point light render spherer");
	ShapeCreator::MakeSphere(sphere, 16, 16, 1.0f);
	sphere->Finalize(game->GetGraphics()->GetWindow());

	/* Initialize the default material. */
	defMaterial = MaterialBP::CreateNoInfo(game->GetLoader());
}

void Plutonium::DeferredRendererBP::InitGsPass(void)
{
	gspass.shdr = new Shader(STATIC_GPASS_VRTX_SHDR_SRC, GPASS_FRAG_SHDR_SRC);
	gspass.matProj = gspass.shdr->GetUniform("Projection");
	gspass.matview = gspass.shdr->GetUniform("View");
	gspass.matMdl = gspass.shdr->GetUniform("Model");
	gspass.specExp = gspass.shdr->GetUniform("Object.SpecularExponent");
	gspass.mapAmbi = gspass.shdr->GetUniform("Object.Ambient");
	gspass.mapDiff = gspass.shdr->GetUniform("Object.Diffuse");
	gspass.mapSpec = gspass.shdr->GetUniform("Object.Specular");
	gspass.mapAlpha = gspass.shdr->GetUniform("Object.Opacity");
	gspass.mapBump = gspass.shdr->GetUniform("Object.Normal");
	gspass.pos = gspass.shdr->GetAttribute("Position");
	gspass.norm = gspass.shdr->GetAttribute("Normal");
	gspass.tan = gspass.shdr->GetAttribute("Tangent");
	gspass.uv = gspass.shdr->GetAttribute("Uv");
}

void Plutonium::DeferredRendererBP::InitGdPass(void)
{
	gdpass.shdr = new Shader(ANIMATED_GPASS_VRTX_SHDR_SRC, GPASS_FRAG_SHDR_SRC);
	gdpass.matProj = gdpass.shdr->GetUniform("Projection");
	gdpass.matView = gdpass.shdr->GetUniform("View");
	gdpass.matMdl = gdpass.shdr->GetUniform("Model");
	gdpass.amnt = gdpass.shdr->GetUniform("Blending");
	gdpass.specExp = gdpass.shdr->GetUniform("Object.SpecularExponent");
	gdpass.mapAmbi = gdpass.shdr->GetUniform("Object.Ambient");
	gdpass.mapDiff = gdpass.shdr->GetUniform("Object.Diffuse");
	gdpass.mapSpec = gdpass.shdr->GetUniform("Object.Specular");
	gdpass.mapAlpha = gdpass.shdr->GetUniform("Object.Opacity");
	gdpass.mapBump = gdpass.shdr->GetUniform("Object.Normal");
	gdpass.pos = gdpass.shdr->GetAttribute("Position1");
	gdpass.norm = gdpass.shdr->GetAttribute("Normal1");
	gdpass.tan = gdpass.shdr->GetAttribute("Tangent1");
	gdpass.pos2 = gdpass.shdr->GetAttribute("Position2");
	gdpass.norm2 = gdpass.shdr->GetAttribute("Normal2");
	gdpass.tan2 = gdpass.shdr->GetAttribute("Tangent2");
	gdpass.uv = gdpass.shdr->GetAttribute("Uv");
}

void Plutonium::DeferredRendererBP::InitDsPass(void)
{
	dspass.shdr = new Shader(DSPASS_VRTX_SHDR_SRC, DSPASS_FRAG_SHDR_SRC);
	dspass.matLs = dspass.shdr->GetUniform("CascadeSpace");
	dspass.matMdl = dspass.shdr->GetUniform("Model");
	dspass.mapAlpha = dspass.shdr->GetUniform("Opacity");
	dspass.mapDiff = dspass.shdr->GetUniform("Diffuse");
	dspass.amnt = dspass.shdr->GetUniform("Blending");
	dspass.pos1 = dspass.shdr->GetAttribute("Position1");
	dspass.pos2 = dspass.shdr->GetAttribute("Position2");
	dspass.uv = dspass.shdr->GetAttribute("Uv");
}

void Plutonium::DeferredRendererBP::InitDPass(void)
{
	dpass.shdr = new Shader(PASS_VRTX_SHDR_SRC, DPASS_FRAG_SHDR_SRC);
	dpass.normSpec = dpass.shdr->GetUniform("Geometry.NormalSpecular");
	dpass.posSpec = dpass.shdr->GetUniform("Geometry.PositionSpecular");
	dpass.ambi = dpass.shdr->GetUniform("Geometry.Ambient");
	dpass.diff = dpass.shdr->GetUniform("Geometry.Diffuse");
	dpass.dir = dpass.shdr->GetUniform("Light.Direction");
	dpass.clrAmbi = dpass.shdr->GetUniform("Light.Ambient");
	dpass.clrDiff = dpass.shdr->GetUniform("Light.Diffuse");
	dpass.clrSpec = dpass.shdr->GetUniform("Light.Specular");
	dpass.matCasc1 = dpass.shdr->GetUniform("Light.CascadeSpace1");
	dpass.matCasc2 = dpass.shdr->GetUniform("Light.CascadeSpace2");
	dpass.matCasc3 = dpass.shdr->GetUniform("Light.CascadeSpace3");
	dpass.end1 = dpass.shdr->GetUniform("Light.CascadeEnd1");
	dpass.end2 = dpass.shdr->GetUniform("Light.CascadeEnd2");
	dpass.end3 = dpass.shdr->GetUniform("Light.CascadeEnd3");
	dpass.shdw1 = dpass.shdr->GetUniform("Light.Cascade1");
	dpass.shdw2 = dpass.shdr->GetUniform("Light.Cascade2");
	dpass.shdw3 = dpass.shdr->GetUniform("Light.Cascade3");
	dpass.matView = dpass.shdr->GetUniform("View");
	dpass.multCasc = dpass.shdr->GetUniform("CascadeIntensity");
	dpass.camPos = dpass.shdr->GetUniform("ViewPosition");
	dpass.pos = dpass.shdr->GetAttribute("Position");
	dpass.uv = dpass.shdr->GetAttribute("ScreenCoordinate");
}

void Plutonium::DeferredRendererBP::InitPPass(void)
{
	ppass.shdr = new Shader(PPASS_VRTX_SHDR_SRC, PPASS_FRAG_SHDR_SRC);
	ppass.matProj = ppass.shdr->GetUniform("Projection");
	ppass.matView = ppass.shdr->GetUniform("View");
	ppass.matMdl = ppass.shdr->GetUniform("Model");
	ppass.screen = ppass.shdr->GetUniform("ScreenSize");
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
	ppass.pos = ppass.shdr->GetAttribute("Position");
}

void Plutonium::DeferredRendererBP::InitFPass(void)
{
	fpass.shdr = new Shader(PASS_VRTX_SHDR_SRC, FPASS_FRAG_SHDR_SRC);
	fpass.screen = fpass.shdr->GetUniform("HdrBuffer");
	fpass.gamma = fpass.shdr->GetUniform("GammaCorrection");
	fpass.exposure = fpass.shdr->GetUniform("Exposure");
	fpass.pos = fpass.shdr->GetAttribute("Position");
	fpass.uv = fpass.shdr->GetAttribute("ScreenCoordinate");
}

void Plutonium::DeferredRendererBP::InitWire(void)
{
	wire.shdr = new Shader(WIRE_VRTX_SHDR_SRC, WIRE_GEOM_SHDR_SRC, WIRE_FRAG_SHDR_SRC);
	wire.matProj = wire.shdr->GetUniform("Projection");
	wire.matView = wire.shdr->GetUniform("View");
	wire.matMdl = wire.shdr->GetUniform("Model");
	wire.clr = wire.shdr->GetUniform("Color");
	wire.mapDiff = wire.shdr->GetUniform("Diffuse");
	wire.amnt = wire.shdr->GetUniform("Blending");
	wire.pos = wire.shdr->GetAttribute("Position");
	wire.pos2 = wire.shdr->GetAttribute("Position2");
	wire.uv = wire.shdr->GetAttribute("Uv");
}

void Plutonium::DeferredRendererBP::InitWNormal(void)
{
	wn.shdr = new Shader(PASS_VRTX_SHDR_SRC, WN_FRAG_SHDR_SRC);
	wn.normSpec = wn.shdr->GetUniform("NormalSpecular");
	wn.pos = wn.shdr->GetAttribute("Position");
	wn.uv = wn.shdr->GetAttribute("ScreenCoordinate");
}

void Plutonium::DeferredRendererBP::InitAlbedo(void)
{
	al.shdr = new Shader(PASS_VRTX_SHDR_SRC, AL_FRAG_SHDR_SRC);
	al.diff = al.shdr->GetUniform("Diffuse");
	al.pos = al.shdr->GetAttribute("Position");
	al.uv = al.shdr->GetAttribute("ScreenCoordinate");
}
#pragma endregion

void Plutonium::DeferredRendererBP::RenderNormal(const Camera * cam)
{
	/* Add directional lights to the scene. */
	for (size_t i = 0; i < queuedDLights.size(); i++)
	{
		const DirectionalLight *light = queuedDLights.at(i);
		std::vector<float> ends;
		Matrix spaces[CASCADE_CNT];

		/* Render shadow map. */
		BeginDirShadowPass();
		RenderDirLightShadow(cam, light, spaces, &ends);
		dspass.shdr->End();

		/* Render lighting. */
		BeginDirLightPass(cam->GetPosition());
		RenderDirLight(cam->GetView(), light, spaces, &ends);
		dpass.shdr->End();
	}

	/* Add point lights to the scene. */
	BeginPntLightPass(cam->GetProjection(), cam->GetView(), cam->GetPosition());
	for (size_t i = 0; i < queuePLights.size(); i++)
	{
		const PointLight *light = queuePLights.at(i);

		/* Only render the light if it's light can be viewed. */
		if (cam->GetClip()->IntersectionSphere(light->Position, light->GetRadius()))
		{
			RenderPntLight(light);
		}
	}
	ppass.shdr->End();
}

void Plutonium::DeferredRendererBP::RenderWireframe(const Matrix & projection, const Matrix & view)
{
	/* Set global uniforms and initialize mix amounf to zero for static model pass. */
	wire.shdr->Begin();
	wire.matProj->Set(projection);
	wire.matView->Set(view);
	wire.amnt->Set(0.0f);

	/* Render static models. */
	for (size_t i = 0; i < queuedModels.size(); i++)
	{
		/* Set model matrix. */
		const StaticObject *model = queuedModels.at(i);
		wire.matMdl->Set(model->GetWorld());

		/* Render all underlying shapes. */
		const std::vector<StaticModel::Shape> *shapes = model->GetModel()->GetShapes();
		for (size_t j = 0; j < shapes->size(); j++)
		{
			MaterialBP *material = shapes->at(j).Material;
			if (material->Visible)
			{
				/* Set color to the designated debug color on debug mode or red on release mode. */
#if defined (DEBUG)
				wire.clr->Set(shapes->at(j).Material->Debug);
#else
				wire.clr->Set(Color::Red());
#endif

				wire.mapDiff->Set(material->Diffuse);

				/* Set first and second position to the same mesh. */
				Buffer *mesh = shapes->at(j).Mesh->GetVertexBuffer();
				mesh->Bind();
				wire.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
				wire.pos2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
				wire.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

				/* Render shape. */
				glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh->GetElementCount()));
			}
		}
	}

	/* Render dynamic models. */
	for (size_t i = 0; i < queuedAnimations.size(); i++)
	{
		/* Set model matrix and mix amount. */
		const DynamicObject *model = queuedAnimations.at(i);
		wire.matMdl->Set(model->GetWorld());
		wire.amnt->Set(model->GetMixAmount());

		/* Set color to the designated debug color on debug mode or yellow on release mode. */
#if defined (DEBUG)
		wire.clr->Set(model->GetModel()->GetMaterial()->Debug);
#else
		wire.clr->Set(Color::Yellow());
#endif

		wire.mapDiff->Set(model->GetModel()->GetMaterial()->Diffuse);

		/* Set first position to the current frame. */
		Buffer *mesh1 = model->GetCurrentFrame()->GetVertexBuffer();
		mesh1->Bind();
		wire.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

		/* Set second position to the next frame. */
		Buffer *mesh2 = model->GetNextFrame()->GetVertexBuffer();
		mesh2->Bind();
		wire.pos2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

		wire.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh1->GetElementCount()));
	}

	wire.shdr->End();
}

void Plutonium::DeferredRendererBP::RenderWorldNormals(void)
{
	/* Setup shader. */
	wn.shdr->Begin();
	wn.normSpec->Set(normalSpec);

	/* Set viewing plane. */
	plane->Bind();
	wn.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	wn.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));

	/* Render and end shader. */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(plane->GetElementCount()));
	wn.shdr->End();
}

void Plutonium::DeferredRendererBP::RenderAlbedo(void)
{
	/* Setup shader. */
	al.shdr->Begin();
	al.diff->Set(diffuse);

	/* Set viewing plane. */
	plane->Bind();
	al.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	al.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));

	/* Render and end shader. */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(plane->GetElementCount()));
	al.shdr->End();
}

void Plutonium::DeferredRendererBP::BeginGsPass(const Matrix & proj, const Matrix & view)
{
	/* Set geometry pass global parameters. */
	gspass.shdr->Begin();
	gspass.matProj->Set(proj);
	gspass.matview->Set(view);
}

void Plutonium::DeferredRendererBP::BeginGdPass(const Matrix & proj, const Matrix & view)
{
	/* Set geometry pass global parameters. */
	gdpass.shdr->Begin();
	gdpass.matProj->Set(proj);
	gdpass.matView->Set(view);
}

void Plutonium::DeferredRendererBP::BeginDirShadowPass(void)
{
	GraphicsAdapter *device = game->GetGraphics();

	/* Setup OpenGL and shader. */
	device->SetRenderTarget(dshdFbo);
	device->SetDepthOuput(true);
	dspass.shdr->Begin();
}

void Plutonium::DeferredRendererBP::BeginDirLightPass(Vector3 camPos)
{
	GraphicsAdapter *device = game->GetGraphics();

	/* Setup OpenGL and shader. */
	device->SetRenderTarget(hdrFbo);
	device->SetColorBlendFunction(BlendState::Additive);
	device->SetBlend(BlendType::One, BlendType::One);
	device->SetDepthOuput(false);
	dpass.shdr->Begin();

	/* Setup global uniforms. */
	dpass.camPos->Set(camPos);
	dpass.normSpec->Set(normalSpec);
	dpass.posSpec->Set(posSpec);
	dpass.ambi->Set(ambient);
	dpass.diff->Set(diffuse);
	dpass.multCasc->Set(DisplayType != RenderType::Shadows ? 0.0f : 0.75f);

	/* Setup mesh. */
	plane->Bind();
	dpass.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	dpass.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));
}

void Plutonium::DeferredRendererBP::BeginPntLightPass(const Matrix & proj, const Matrix & view, Vector3 camPos)
{
	ppass.shdr->Begin();

	/* Setup global uniforms. */
	ppass.matProj->Set(proj);
	ppass.matView->Set(view);
	ppass.camPos->Set(camPos);
	ppass.screen->Set(game->GetGraphics()->GetWindow()->GetClientBounds().Size);
	ppass.normSpec->Set(normalSpec);
	ppass.posSpec->Set(posSpec);
	ppass.ambi->Set(ambient);
	ppass.diff->Set(diffuse);

	/* Setup mesh. */
	sphere->GetVertexBuffer()->Bind();
	ppass.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
}

void Plutonium::DeferredRendererBP::SetCascadeMax(const Camera * cam, float max[CASCADE_CNT])
{
	for (size_t i = 1; i < CASCADE_CNT + 1; i++)
	{
		/* Get current amount value for logarithmic and linear scale. */
		float a = static_cast<float>(i) / static_cast<float>(CASCADE_CNT);

		/* Get current logarithmic and linear(uniform) value. */
		float log = cam->GetNear() * powf(cam->GetFar(), a);
		float uni = lerp(cam->GetNear(), cam->GetFar(), a);

		/* Mix them using the lambda as the result. */
		max[i - 1] = CascadeLambda * log + (1.0f - CascadeLambda) * uni;
	}
}

float Plutonium::DeferredRendererBP::GetFurthestZFromCam(const Camera * cam)
{
	Box scene;

	/* Add all static models to the scene bounding box if they cast a shadow. */
	for (size_t i = 0; i < queuedModels.size(); i++)
	{
		const StaticObject *cur = queuedModels.at(i);
		if (cur->CastsShadows) scene = Box::Merge(scene, cur->GetBoundingBox());
	}

	/* Add all animated models to the scene bounding box if they cast a shadow. */
	for (size_t i = 0; i < queuedAnimations.size(); i++)
	{
		const DynamicObject *cur = queuedAnimations.at(i);
		if (cur->CastsShadows) scene = Box::Merge(scene, cur->GetBoundingBox());
	}

	/* Calculate the furthest z coordinate in view space. */
	float furthest = cam->GetNear();
	for (size_t i = 0; i < 8; i++)
	{
		Vector3 corner = scene[i];
		float z = -(cam->GetView() * corner).Z;
		furthest = max(furthest, z);
	}

	/* Return the minimum of the cull distance and the bounding box max to ensure we don't render behind the frustum. */
	return min(furthest, cam->GetFar());
}

void Plutonium::DeferredRendererBP::SetCascadeEnds(const Camera * cam, std::vector<float>* ends)
{
	float furthest = GetFurthestZFromCam(cam);
	float max[CASCADE_CNT];
	SetCascadeMax(cam, max);

	/* Calculate how many cascades are actually needed (might save a shadow map render). */
	size_t cascadeCnt = CASCADE_CNT; //1;
	//for (size_t i = 0; i < CASCADE_CNT; i++)
	//{
	//	if (furthest >= max[i]) ++cascadeCnt;
	//}

	/* Calculate the needed ends of the frustum. */
	ends->push_back(cam->GetNear());
	for (size_t i = 1; i < cascadeCnt; i++)
	{
		float a = static_cast<float>(i) / static_cast<float>(cascadeCnt);
		float log = cam->GetNear() * powf(furthest / cam->GetNear(), a);
		float uni = lerp(cam->GetNear(), furthest, a);
		ends->push_back(CascadeLambda * log + (1.0f - CascadeLambda) * uni);
	}
	ends->push_back(furthest);
}

Plutonium::Box Plutonium::DeferredRendererBP::CalcOrthoBox(const Camera * cam, float near, float far)
{
	/* Gets the camera tangent of the camera fov and the aspect ratio. */
	float tanFov = tanf(cam->GetFov() * 0.5f);
	float aspr = game->GetGraphics()->GetWindow()->AspectRatio();

	/* Calculate the height and width of the near and far plane. */
	float nh = tanFov * near;
	float nw = nh * aspr;
	float fh = tanFov * far;
	float fw = fh * aspr;

	/* Get the camera coordinate system. */
	Vector3 right = cam->GetOrientation().GetRight();
	Vector3 up = cam->GetOrientation().GetUp();
	Vector3 back = cam->GetOrientation().GetForward();

	/* Get the center of the near and far plane. */
	Vector3 nc = cam->GetPosition() + back * near;
	Vector3 fc = cam->GetPosition() + back * far;

	/* Calculate the orthographic box corners. */
	Box result;
	result = Box::Merge(result, nc - right * nw - up * nh);
	result = Box::Merge(result, nc - right * nw + up * nh);
	result = Box::Merge(result, nc + right * nw - up * nh);
	result = Box::Merge(result, nc + right * nw + up * nh);
	result = Box::Merge(result, fc - right * fw - up * fh);
	result = Box::Merge(result, fc - right * fw + up * fh);
	result = Box::Merge(result, fc + right * fw - up * fh);
	result = Box::Merge(result, fc + right * fw + up * fh);
	return result;
}

Plutonium::Matrix Plutonium::DeferredRendererBP::CalcDirLightVP(const Camera * cam, const Box & frustum, const DirectionalLight * light)
{
	/* Create view matrix. */
	Vector3 pos = frustum.GetCenter();
	Matrix view = Matrix::CreateLookAt(pos, pos + light->GetDirection() * LightOffset, light->GetUp());

	/* Create orthographic projection matrix. */
	float d = max(frustum.Size.X, frustum.Size.Z);
	Matrix proj = Matrix::CreateOrtho(d, d, cam->GetNear(), cam->GetFar());

	/* Return light space matrix. */
	return proj * view;
}

void Plutonium::DeferredRendererBP::RenderModel(const Camera * cam, const StaticObject * model, const MaterialBP * overrideMaterial)
{
	/* Only render the model if it's visible by the camera. */
	Box bb = model->GetBoundingBox();
	if (cam->GetClip()->IntersectionBox(bb.Position, bb.Size))
	{
		/* Set model matrix. */
		gspass.matMdl->Set(model->GetWorld());

		/* Loops through all shapes in the model. */
		const std::vector<StaticModel::Shape> *shapes = model->GetModel()->GetShapes();
		for (size_t i = 0; i < shapes->size(); i++)
		{
			/* Render the current material if it's visible. */
			const MaterialBP *material = shapes->at(i).Material;
			if (material->Visible)
			{
				/* Only render the mesh if it's inside of the camera view. */
				Mesh *mesh = shapes->at(i).Mesh;
				bb = mesh->GetBoundingBox() * model->GetWorld();
				if (cam->GetClip()->IntersectionBox(bb.Position, bb.Size))
				{
					Buffer *buffer = mesh->GetVertexBuffer();

					/* Set material attributes. */
					if (!overrideMaterial)
					{
						gspass.mapAmbi->Set(material->Ambient);
						gspass.mapDiff->Set(material->Diffuse);
						gspass.mapSpec->Set(material->Specular);
						gspass.specExp->Set(material->SpecularExponent);
					}
					else
					{
						gspass.mapAmbi->Set(overrideMaterial->Ambient);
						gspass.mapDiff->Set(overrideMaterial->Diffuse);
						gspass.mapSpec->Set(overrideMaterial->Specular);
						gspass.specExp->Set(overrideMaterial->SpecularExponent);
					}
					gspass.mapAlpha->Set(material->Opacity);
					gspass.mapBump->Set(material->Normal);

					/* Set mesh attributes. */
					buffer->Bind();
					gspass.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
					gspass.norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
					gspass.tan->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
					gspass.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

					/* Render the shape to the GBuffer. */
					glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
				}
			}
		}
	}
}

void Plutonium::DeferredRendererBP::RenderModel(const Camera * cam, const DynamicObject * model, const MaterialBP * overrideMaterial)
{
	/* Only render the model if it's visible by the camera. */
	Box bb = model->GetBoundingBox();
	if (cam->GetClip()->IntersectionBox(bb.Position, bb.Size))
	{
		/* Set model matrix and inter frame mix amount. */
		gdpass.matMdl->Set(model->GetWorld());
		gdpass.amnt->Set(model->GetMixAmount());

		const MaterialBP *material = model->GetModel()->GetMaterial();
		if (material->Visible)
		{
			Buffer *curMesh = model->GetCurrentFrame()->GetVertexBuffer();
			Buffer *nextMesh = model->GetNextFrame()->GetVertexBuffer();

			/* Set material attributes. */
			if (!overrideMaterial)
			{
				gdpass.mapAmbi->Set(material->Ambient);
				gdpass.mapDiff->Set(material->Diffuse);
				gdpass.mapSpec->Set(material->Specular);
				gdpass.specExp->Set(material->SpecularExponent);
			}
			else
			{
				gdpass.mapAmbi->Set(overrideMaterial->Ambient);
				gdpass.mapDiff->Set(overrideMaterial->Diffuse);
				gdpass.mapSpec->Set(overrideMaterial->Specular);
				gdpass.specExp->Set(overrideMaterial->SpecularExponent);
			}
			gdpass.mapAlpha->Set(material->Opacity);
			gdpass.mapBump->Set(material->Normal);

			/* Set first mesh attributes. */
			curMesh->Bind();
			gdpass.pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
			gdpass.norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
			gdpass.tan->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
			gdpass.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

			/* Set second mesh attributes. */
			nextMesh->Bind();
			gdpass.pos2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
			gdpass.norm2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
			gdpass.tan2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));

			/* Render the shape to the GBuffer. */
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(curMesh->GetElementCount()));
		}
	}
}

#include "Core\String.h"

void Plutonium::DeferredRendererBP::RenderDirLightShadow(const Camera * cam, const DirectionalLight * light, Matrix * spaces, std::vector<float> * ends)
{
	SetCascadeEnds(cam, ends);

	/* Loop through cascades. */
	for (size_t i = 0, j = 1; j < ends->size(); i++, j++)
	{
		spaces[i] = CalcDirLightVP(cam, CalcOrthoBox(cam, ends->at(i), ends->at(j)), light);

		/* Set current cascade as the writing target and clear it. */
		dshdFbo->BindForWriting(cascades[i]);
		game->GetGraphics()->Clear(ClearTarget::Depth);

		/* Only render to the depth buffer if the light is allowed to create shadows. */
		if (light->CreatesShadows)
		{
			dspass.matLs->Set(spaces[i]);
			dspass.amnt->Set(0.0f);

			/* Render all static models to depth map. */
			for (size_t i = 0; i < queuedModels.size(); i++)
			{
				/* Only render the model if it is allowed to cast a shadow. */
				const StaticObject *model = queuedModels.at(i);
				if (!model->CastsShadows) continue;

				/* Set model matrix. */
				dspass.matMdl->Set(model->GetWorld());

				/* Loop through all shapes in the mode. */
				const std::vector<StaticModel::Shape> *shapes = model->GetModel()->GetShapes();
				for (size_t j = 0; j < shapes->size(); j++)
				{
					const MaterialBP *material = shapes->at(j).Material;
					if (material->Visible)
					{
						Buffer *buffer = shapes->at(j).Mesh->GetVertexBuffer();

						/* Set material attributes. */
						dspass.mapDiff->Set(material->Diffuse);
						dspass.mapAlpha->Set(material->Opacity);

						/* Set mesh attributes. */
						buffer->Bind();
						dspass.pos1->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
						dspass.pos2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
						dspass.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

						/* Render the shape to the shadow depth buffer. */
						glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
					}
				}
			}

			/* Render all dynamic models to depth map. */
			for (size_t i = 0; i < queuedAnimations.size(); i++)
			{
				/* Only render the model if it is allowed to cast a shadow. */
				const DynamicObject *model = queuedAnimations.at(i);
				if (!model->CastsShadows) continue;

				/* Set model matrix. */
				dspass.matMdl->Set(model->GetWorld());
				dspass.amnt->Set(model->GetMixAmount());

				const MaterialBP *material = model->GetModel()->GetMaterial();
				if (material->Visible)
				{
					Buffer *cur = model->GetCurrentFrame()->GetVertexBuffer();
					Buffer *next = model->GetNextFrame()->GetVertexBuffer();

					/* Set material attributes. */
					dspass.mapDiff->Set(material->Diffuse);
					dspass.mapAlpha->Set(material->Opacity);

					/* Set mesh attributes. */
					cur->Bind();
					dspass.pos1->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
					dspass.uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

					next->Bind();
					dspass.pos2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

					/* Render the shape to the shadow depth buffer. */
					glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cur->GetElementCount()));
				}
			}
		}
	}
}

void Plutonium::DeferredRendererBP::RenderDirLight(const Matrix & view, const DirectionalLight * light, Matrix * spaces, std::vector<float> * ends)
{
	dpass.dir->Set(-normalize(light->GetDirection()));
	dpass.clrAmbi->Set(light->Ambient);
	dpass.clrDiff->Set(light->Diffuse);
	dpass.clrSpec->Set(light->Specular);
	dpass.matView->Set(view);
	dpass.matCasc1->Set(spaces[0]);
	dpass.matCasc2->Set(spaces[1]);
	dpass.matCasc3->Set(spaces[2]);
	dpass.end1->Set(ends->at(1));
	dpass.end2->Set(ends->at(2));
	dpass.end3->Set(ends->at(3));
	dpass.shdw1->Set(cascades[0]);
	dpass.shdw2->Set(cascades[1]);
	dpass.shdw3->Set(cascades[2]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(plane->GetElementCount()));
}

void Plutonium::DeferredRendererBP::RenderPntLight(const PointLight * light)
{
	ppass.matMdl->Set(Matrix::CreateTranslation(light->Position) * Matrix::CreateScalar(light->GetRadius()));
	ppass.lpos->Set(light->Position);
	ppass.c->Set(light->Constant);
	ppass.l->Set(light->Linear);
	ppass.q->Set(light->Quadratic);
	ppass.clrAmbi->Set(light->Ambient);
	ppass.clrDiff->Set(light->Diffuse);
	ppass.clrSpec->Set(light->Specular);

	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(sphere->GetVertexBuffer()->GetElementCount()));
}

void Plutonium::DeferredRendererBP::FixForMonitor(void)
{
	/* Start shader. */
	fpass.shdr->Begin();

	/* Set uniforms. */
	fpass.screen->Set(screen);
	fpass.gamma->Set(game->GetGraphics()->GetWindow()->GetGraphicsDevice().GammaCorrection);
	fpass.exposure->Set(Exposure);

	/* Setup screen quad (use dpass quad plane). */
	plane->Bind();
	fpass.pos->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Position));
	fpass.uv->Initialize(false, sizeof(PlaneVertexFormat), offset_ptr(PlaneVertexFormat, Uv));

	/* Fix for monitor for each fragment and end shader. */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(plane->GetElementCount()));
	fpass.shdr->End();
}