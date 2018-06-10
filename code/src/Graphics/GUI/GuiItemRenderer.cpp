#include "Graphics\GUI\GuiItemRenderer.h"

Plutonium::GuiItemRenderer::GuiItemRenderer(GraphicsAdapter * device)
	: device(device)
{
	/* Create the default GuiItem background texture. */
	defBackTex = new Texture(1, 1, device->GetWindow(), &TextureCreationOptions::DefaultNoMipMap, "DefaultGuiItemBackground");
	defBackTex->SetData(Color::White.ToArray());

	/* Make sure the projection matrix stays up to date. */
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
	device->GetWindow()->SizeChanged.Add(this, &GuiItemRenderer::WindowResizeEventHandler);

	/* Initialize shaders. */
	InitBasicShader();
}

Plutonium::GuiItemRenderer::~GuiItemRenderer(void)
{
	device->GetWindow()->SizeChanged.Remove(this, &GuiItemRenderer::WindowResizeEventHandler);
	delete_s(defBackTex);
	delete_s(basic.shdr);
}

void Plutonium::GuiItemRenderer::RenderGuiItem(Rectangle bounds, float rounding, float orientation, Color backColor, TextureHandler background, bool shouldDrawResizer, const Buffer *mesh)
{
	/* Make sure to convert the position to OpenGL coordinates and add a default background (white color) if none is specified. */
	BasicGuiItemArgs args =
	{
		mesh,
		device->ToOpenGL(bounds.Position),
		bounds.Size,
		orientation,
		rounding,
		backColor,
		shouldDrawResizer ? Color::White : Color::Transparent,
		background ? background : defBackTex
	};

	basicDrawQueue.push(args);
}

void Plutonium::GuiItemRenderer::End(bool noBlending)
{
	/* Enable blending (default). */
	if (!noBlending) device->SetAlphaSourceBlend(BlendType::ISrcAlpha);

	/* Renders all basic GuiItems. */
	RenderBasics();
}

void Plutonium::GuiItemRenderer::RenderBasics(void)
{
	/* Set shader globals. */
	basic.shdr->Begin();
	basic.matProj->Set(projection);

	/* Render all queued items. */
	while (basicDrawQueue.size() > 0)
	{
		BasicGuiItemArgs cur = basicDrawQueue.back();
		basicDrawQueue.pop();

		/* Create or bus uniforms to shader. */
		basic.matMdl->Set(Matrix::CreateWorld(cur.Position, cur.Orientation, Vector2::One));
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

void Plutonium::GuiItemRenderer::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	/* Update the projection matrix. */
	const Rectangle vp = sender->GetClientBounds();
	projection = Matrix::CreateOrtho(vp.Position.X, vp.Size.X, vp.Position.Y, vp.Size.Y, 0.0f, 1.0f);
}

void Plutonium::GuiItemRenderer::InitBasicShader(void)
{
	constexpr const char *VRTX_SHDR_SRC =
		"#version 430 core																\n"

		"uniform mat4 projection;														\n"
		"uniform mat4 model;															\n"

		"in vec4 posUv;																	\n"

		"out vec2 uv;																	\n"

		"void main()																	\n"
		"{																				\n"
		"	uv = posUv.zw;																\n"
		"	gl_Position = projection * model * vec4(posUv.xy, 0.0f, 1.0f);				\n"
		"}";

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
		"	if (RoundRect(gl_FragCoord.xy - center) < 0.0f)								\n"
		"	{																			\n"
		"		vec4 texel = texture(background, uv);									\n"
		"		fragColor = texel * backgroundFilter;									\n"
		"	}																			\n"
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