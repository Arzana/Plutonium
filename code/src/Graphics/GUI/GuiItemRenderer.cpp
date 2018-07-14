#include "Graphics\GUI\GuiItemRenderer.h"
#include "Core\StringFunctions.h"

Plutonium::GuiItemRenderer::GuiItemRenderer(GraphicsAdapter * device)
	: device(device)
{
	/* Create the default GuiItem background texture. */
	defBackTex = new Texture(1, 1, device->GetWindow(), &TextureCreationOptions::DefaultNoMipMap, "DefaultGuiItemBackground");
	defBackTex->SetData(Color::White().ToArray());

	/* Make sure the projection matrix stays up to date. */
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
	device->GetWindow()->SizeChanged.Add(this, &GuiItemRenderer::WindowResizeEventHandler);

	/* Initialize shaders. */
	InitBasicShader();
	InitTextShader();
	InitBarShader();
}

Plutonium::GuiItemRenderer::~GuiItemRenderer(void)
{
	device->GetWindow()->SizeChanged.Remove(this, &GuiItemRenderer::WindowResizeEventHandler);
	delete_s(defBackTex);
	delete_s(basic.shdr);
}

void Plutonium::GuiItemRenderer::RenderBackground(Rectangle bounds, float rounding, Color backColor, TextureHandler background, const Buffer *mesh, bool lateCall)
{
	/* Make sure to convert the position to OpenGL coordinates and add a default background (white color) if none is specified. */
	BasicGuiItemArgs args =
	{
		mesh,
		device->ToOpenGL(bounds.Position),
		bounds.Size,
		rounding,
		backColor,
		background ? background : defBackTex
	};

	if(lateCall) basicLateDrawQueue.push(args);
	else basicEarlyDrawQueue.push(args);
}

void Plutonium::GuiItemRenderer::RenderTextForeground(Vector2 position, Color textColor, const Font * font, const Buffer * mesh)
{
	/* Make sure to convert the position to OpenGL coordinates. */
	LabelTextArgs args =
	{
		mesh,
		font,
		device->ToOpenGL(position),
		textColor
	};

	textDrawQueue.push(args);
}

void Plutonium::GuiItemRenderer::RenderBarForeground(Vector2 position, Rectangle parentBounds, float parentRounding, Color barColor, TextureHandler texture, const Buffer * mesh)
{
	/* Make sure we convert the position to OpenGL coordinates. */
	ProgressBarBarArgs args =
	{
		mesh, 
		device->ToOpenGL(position),
		device->ToOpenGL(parentBounds.Position),
		parentBounds.Size,
		parentRounding,
		barColor,
		texture ? texture : defBackTex
	};

	barDrawQueue.push(args);
}

void Plutonium::GuiItemRenderer::End(bool noBlending)
{
	/* Enable blending (default). */
	if (!noBlending) device->SetAlphaSourceBlend(BlendType::ISrcAlpha);

	/* Renders all queued GuiItems. */
	RenderBasics(basicEarlyDrawQueue);
	RenderText();
	RenderBars();
	RenderBasics(basicLateDrawQueue);
}

void Plutonium::GuiItemRenderer::RenderBasics(std::queue<BasicGuiItemArgs> &queue)
{
	/* Set shader globals. */
	basic.shdr->Begin();
	basic.matProj->Set(projection);

	/* Render all queued items. */
	while (queue.size() > 0)
	{
		BasicGuiItemArgs cur = queue.front();
		queue.pop();

		/* Create or bus uniforms to shader. */
		basic.matMdl->Set(Matrix::CreateTranslation(cur.Position.X, cur.Position.Y, 0.0f));
		basic.background->Set(cur.Background);
		basic.clrBack->Set(cur.BackgroundColor);
		basic.pos->Set(cur.Position);
		basic.size->Set(cur.Size);
		basic.rounding->Set(cur.Rounding);

		/* Set position/uv attribute. */
		cur.Mesh->Bind();
		basic.posUv->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

		/* Render GuiItem. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cur.Mesh->GetElementCount()));
	}

	basic.shdr->End();
}

void Plutonium::GuiItemRenderer::RenderText(void)
{
	/* Set shader globals. */
	text.shdr->Begin();
	text.matProj->Set(projection);

	/* Render all queued items. */
	while (textDrawQueue.size() > 0)
	{
		LabelTextArgs cur = textDrawQueue.front();
		textDrawQueue.pop();

		/* Create or bus uniforms to shader. */
		text.matMdl->Set(Matrix::CreateTranslation(cur.Position.X, cur.Position.Y, 0.0f));
		text.map->Set(cur.Font->map);
		text.clr->Set(cur.TextColor);
		
		/* Set position/uv attribute. */
		cur.Mesh->Bind();
		text.posUv->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

		/* Render text. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cur.Mesh->GetElementCount()));
	}

	text.shdr->End();
}

void Plutonium::GuiItemRenderer::RenderBars(void)
{
	/* Set shader globals. */
	bar.shdr->Begin();
	bar.matProj->Set(projection);

	/* Render all queued items. */
	while (barDrawQueue.size() > 0)
	{
		ProgressBarBarArgs cur = barDrawQueue.front();
		barDrawQueue.pop();

		/* Create or bus uniforms to shader. */
		bar.matMdl->Set(Matrix::CreateTranslation(cur.Position.X, cur.Position.Y, 0.0f));
		bar.texture->Set(cur.Texture);
		bar.clr->Set(cur.BarColor);
		bar.pos->Set(cur.ParentPosition);
		bar.size->Set(cur.ParentSize);
		bar.rounding->Set(cur.ParentRounding);

		/* Set position/uv attribute. */
		cur.Mesh->Bind();
		basic.posUv->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

		/* Render bar. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cur.Mesh->GetElementCount()));
	}

	bar.shdr->End();
}

void Plutonium::GuiItemRenderer::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	/* Update the projection matrix. */
	const Rectangle vp = sender->GetClientBounds();
	projection = Matrix::CreateOrtho(vp.Position.X, vp.Size.X, vp.Position.Y, vp.Size.Y, 0.0f, 1.0f);
}

/* The vertex shader doesn't change for any shader so we save it globally. */
constexpr const char *VRTX_SHDR_SRC =
	"#version 430 core																	\n"

	"uniform mat4 projection;															\n"
	"uniform mat4 model;																\n"

	"in vec4 posUv;																		\n"

	"out vec2 uv;																		\n"

	"void main()																		\n"
	"{																					\n"
	"	uv = posUv.zw;																	\n"
	"	gl_Position = projection * model * vec4(posUv.xy, 0.0f, 1.0f);					\n"
	"}";

void Plutonium::GuiItemRenderer::InitBasicShader(void)
{
	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																\n"

		"uniform sampler2D background;													\n"
		"uniform vec4 backgroundFilter;													\n"
		"uniform float border;															\n"
		"uniform vec2 size;																\n"
		"uniform vec2 pos;																\n"

		"in vec2 uv;																	\n"

		"out vec4 fragColor;															\n"

		"float RoundRect(vec2 pos)														\n"
		"{																				\n"
		"	vec2 radius = size / 2.0f - border;											\n"
		"	return length(max(abs(pos) - radius, 0.0f)) - border;						\n"
		"}																				\n"

		"void main()																	\n"
		"{																				\n"
		"	vec2 center = pos + vec2(size.x, -size.y) / 2.0f;							\n"
		"	if (RoundRect(gl_FragCoord.xy - center) <= 0.0f)							\n"
		"	{																			\n"
		"		vec4 texel = texture(background, uv);									\n"
		"		fragColor = texel * backgroundFilter;									\n"
		"	}																			\n"
		"	else fragColor = vec4(0.0f);												\n"
		"}";

	basic.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	basic.matProj = basic.shdr->GetUniform("projection");
	basic.matMdl = basic.shdr->GetUniform("model");
	basic.background = basic.shdr->GetUniform("background");
	basic.clrBack = basic.shdr->GetUniform("backgroundFilter");
	basic.rounding = basic.shdr->GetUniform("border");
	basic.pos = basic.shdr->GetUniform("pos");
	basic.size = basic.shdr->GetUniform("size");
	basic.posUv = basic.shdr->GetAttribute("posUv");
}

void Plutonium::GuiItemRenderer::InitTextShader(void)
{
	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																\n"
		
		"uniform sampler2D map;															\n"
		"uniform vec4 color;															\n"
		
		"in vec2 uv;																	\n"
		
		"out vec4 fragColor;															\n"
		
		"void main()																	\n"
		"{																				\n"
		"	fragColor = texture(map, uv) * color;										\n"
		"}";

	text.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	text.matProj = text.shdr->GetUniform("projection");
	text.matMdl = text.shdr->GetUniform("model");
	text.map = text.shdr->GetUniform("map");
	text.clr = text.shdr->GetUniform("color");
	text.posUv = text.shdr->GetAttribute("posUv");
}

void Plutonium::GuiItemRenderer::InitBarShader(void)
{
	constexpr const char *FRAG_SHDR_SRC =
		"#version 430 core																\n"
		
		"uniform sampler2D bar;															\n"
		"uniform vec4 color;															\n"
		"uniform float border;															\n"
		"uniform vec2 size;																\n"
		"uniform vec2 pos;																\n"
		
		"in vec2 uv;																	\n"
		
		"out vec4 fragColor;															\n"

		"float RoundRect(vec2 pos)														\n"
		"{																				\n"
		"	vec2 radius = size / 2.0f - border;											\n"
		"	return length(max(abs(pos) - radius, 0.0f)) - border;						\n"
		"}																				\n"

		"void main()																	\n"
		"{																				\n"
		"	vec2 center = pos + vec2(size.x, -size.y) / 2.0f;							\n"
		"	if (RoundRect(gl_FragCoord.xy - center) <= 0.0f)							\n"
		"	{																			\n"
		"		vec4 texel = texture(bar, uv);											\n"
		"		fragColor = texel * color;												\n"
		"	}																			\n"
		"	else fragColor = vec4(0.0f);												\n"
		"}";

	bar.shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	bar.matProj = bar.shdr->GetUniform("projection");
	bar.matMdl = bar.shdr->GetUniform("model");
	bar.texture = bar.shdr->GetUniform("bar");
	bar.clr = bar.shdr->GetUniform("color");
	bar.rounding = bar.shdr->GetUniform("border");
	bar.pos = bar.shdr->GetUniform("pos");
	bar.size = bar.shdr->GetUniform("size");
	bar.posUv = bar.shdr->GetAttribute("posUv");
}