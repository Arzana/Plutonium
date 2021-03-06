#include "Graphics\GUI\Containers\Menu.h"
#include "Core\StringFunctions.h"
#include "Core\Threading\PuThread.h"

#define LOG_INIT(c)		LOG("Initializing controll(%s).", _CRT_NAMEOF_RAW(c))

using namespace Plutonium;

constexpr size_t MAX_STR_LEN = 64;

Plutonium::Menu::Menu(Game *game)
	: GameComponent(game), Container(), defaultFontIdx(-1),
	loadCnt(0), loadTarget(0), loadLock(), visible(true), callCreate(false)
{
	renderer = new GuiItemRenderer(game->GetGraphics());

#if defined (DEBUG)
	stringVbo = new Buffer(game->GetGraphics()->GetWindow(), BindTarget::Array);
	stringVbo->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, MAX_STR_LEN * 6);
#endif
}

Plutonium::Menu::~Menu(void)
{
	delete_s(renderer);
#if defined (DEBUG)
	delete_s(stringVbo);
#endif
}

void Plutonium::Menu::Hide(void)
{
	Disable();
	visible = false;
}

void Plutonium::Menu::Show(void)
{
	Enable();
	visible = true;
}

int32 Plutonium::Menu::GetScreenWidth(void) const
{
	return static_cast<int32>(game->GetGraphics()->GetWindow()->GetClientBounds().GetWidth());
}

int32 Plutonium::Menu::GetScreenHeight(void) const
{
	return static_cast<int32>(game->GetGraphics()->GetWindow()->GetClientBounds().GetHeight());
}

bool Plutonium::Menu::HasFocus(void) const
{
	return Container::HasFocus();
}

const Font * Plutonium::Menu::GetDefaultFont(void) const
{
	LOG_THROW_IF(defaultFontIdx == -1, "No default font is set!");
	return loadedFonts.at(defaultFontIdx);
}

const Font * Plutonium::Menu::GetFont(const char * name) const
{
	for (size_t i = 0; i < loadedFonts.size(); i++)
	{
		const Font *cur = loadedFonts.at(i);
		if (eqlstr(cur->GetName(), name)) return cur;
	}

	LOG_WAR("Unable to find font '%s'!", name);
	return nullptr;
}

const Texture * Plutonium::Menu::GetTexture(const char * name) const
{
	for (size_t i = 0; i < loadedTextures.size(); i++)
	{
		TextureHandler cur = loadedTextures.at(i);
		if (eqlstr(cur->GetName(), name)) return cur;
	}

	LOG_WAR("Unable to find texture '%s'!", name);
	return nullptr;
}

GuiItem * Plutonium::Menu::AddGuiItem(void)
{
	LOG_INIT(GuiItem);

	GuiItem *result = new GuiItem(game);
	AddItem(result);
	return result;
}

Label * Plutonium::Menu::AddLabel(const Font * font)
{
	LOG_INIT(Label);
	CheckFont(font);

	Label *result = new Label(game, font ? font : loadedFonts.at(defaultFontIdx));
	AddItem(result);
	return result;
}

Button * Plutonium::Menu::AddButton(const Font * font)
{
	LOG_INIT(Button);
	CheckFont(font);

	Button *result = new Button(game, font ? font : loadedFonts.at(defaultFontIdx));
	AddItem(result);
	return result;
}

ProgressBar * Plutonium::Menu::AddProgressBar(void)
{
	LOG_INIT(ProgressBar);

	ProgressBar *result = new ProgressBar(game);
	AddItem(result);
	return result;
}

Slider * Plutonium::Menu::AddSlider(void)
{
	LOG_INIT(Slider);

	Slider *result = new Slider(game);
	AddItem(result);
	return result;
}

TextBox * Plutonium::Menu::AddTextBox(const Font * font)
{
	LOG_INIT(TextBox);
	CheckFont(font);

	TextBox *result = new TextBox(game, font ? font : loadedFonts.at(defaultFontIdx));
	result->GainedFocus.Add(this, &Menu::OnTextBoxGainFocus);
	AddItem(result);
	return result;
}

GUIWindow * Plutonium::Menu::AddWindow(const Font * font)
{
	LOG_INIT(GUIWindow);
	CheckFont(font);

	GUIWindow *result = new GUIWindow(game, font ? font : loadedFonts.at(defaultFontIdx));
	result->Closed.Add(this, &Menu::OnWindowClosed);
	AddItem(result);
	return result;
}

void Plutonium::Menu::SetDefaultFont(const char * path, float size)
{
	LOG_WAR_IF(defaultFontIdx != -1, "Redefining the default font!");
	++loadTarget;
	callCreate = false;

	game->GetLoader()->LoadFont(path, Callback<Font>([&](const AssetLoader*, const Font *result)
	{
		loadLock.lock();
		defaultFontIdx = static_cast<int32>(loadedFonts.size());
		loadedFonts.push_back(result);
		loadLock.unlock();

		++loadCnt;
		CheckIfLoadingDone();
	}), size);
}

void Plutonium::Menu::LoadFont(const char * path, float size)
{
	++loadTarget;
	callCreate = false;

	game->GetLoader()->LoadFont(path, Callback<Font>([&](const AssetLoader*, const Font *result)
	{
		loadLock.lock();
		loadedFonts.push_back(result);
		loadLock.unlock();

		++loadCnt;
		CheckIfLoadingDone();
	}), size);
}

void Plutonium::Menu::LoadTexture(const char * path, const TextureCreationOptions * config)
{
	++loadTarget;
	callCreate = false;

	game->GetLoader()->LoadTexture(path, Callback<Texture>([&](const AssetLoader*, TextureHandler result)
	{
		loadLock.lock();
		loadedTextures.push_back(result);
		loadLock.unlock();

		++loadCnt;
		CheckIfLoadingDone();
	}), false, config);
}

#if defined(DEBUG)
void Plutonium::Menu::DrawString(Vector2 position, const char * text, Color color)
{
	LOG_THROW_IF(defaultFontIdx == -1, "Unable to render debug text if no default font is specified!");

	const Font *font = loadedFonts.at(defaultFontIdx);
	const char32 *string = heapwstr(text);	// TODO: Fix memory leak.
	float lh = static_cast<float>(font->GetLineSpace()) * (cntchar(text, '\n') + 1);

	/* Create CPU side vertices buffer. (6 vertices per quad of vector4 type per glyph). */
	size_t len = strlen(string), size = len * 6;
	Vector4 *vertices = malloca_s(Vector4, size);

	float xAdder = 0.0f;
	float yAdder = -lh;
	for (size_t i = 0, j = 0; i < len; i++)
	{
		/* Get current character. */
		const Character *ch = font->GetCharOrDefault(text[i]);

		/* Defines components of the position vertices. */
		float w = ch->Size.X;
		float h = ch->Size.Y;
		float x = xAdder + ch->Bearing.X;
		float y = yAdder - (ch->Size.Y + ch->Bearing.Y);
		Vector2 tl = ch->Bounds.Position;
		Vector2 br = ch->Bounds.Position + ch->Bounds.Size;

		/* Populate buffer. */
		vertices[j++] = Vector4(x, y + h, tl.X, tl.Y);
		vertices[j++] = Vector4(x, y, tl.X, br.Y);
		vertices[j++] = Vector4(x + w, y, br.X, br.Y);
		vertices[j++] = Vector4(x, y + h, tl.X, tl.Y);
		vertices[j++] = Vector4(x + w, y, br.X, br.Y);
		vertices[j++] = Vector4(x + w, y + h, br.X, tl.Y);

		xAdder += ch->Advance;
		if (ch->Key == U'\n') yAdder -= lh;
	}

	/* Create GPU side buffer. */
	stringVbo->SetData(vertices, size);
	freea_s(vertices);

	/* Render debug string. */
	renderer->RenderTextForeground(position, color, font, stringVbo);
}
#else
void Plutonium::Menu::DrawString(Vector2, const char*, Color)
{
	LOG_WAR_ONCE("Drawing string directly is disabled on debug mode!");
}
#endif

void Plutonium::Menu::Initialize(void)
{
	GameComponent::Initialize();
	Hide();
}

void Plutonium::Menu::Update(float dt)
{
	GameComponent::Update(dt);
	Container::Update(dt);

	/* Call create once the loading is done. */
	if (callCreate)
	{
		callCreate = false;
		Create();
	}
}

void Plutonium::Menu::Render(float dt)
{
	if (visible)
	{
		GameComponent::Render(dt);
		Container::Render(renderer);
		renderer->End();
	}
}

void Plutonium::Menu::Finalize(void)
{
	GameComponent::Finalize();

	for (size_t i = 0; i < loadedFonts.size(); i++)
	{
		game->GetLoader()->Unload(loadedFonts.at(i));
	}

	for (size_t i = 0; i < loadedTextures.size(); i++)
	{
		game->GetLoader()->Unload(loadedTextures.at(i));
	}
}

void Plutonium::Menu::CheckIfLoadingDone(void)
{
	if (loadCnt.load() >= loadTarget.load())
	{
		callCreate = true;
		Show();
	}
}

void Plutonium::Menu::CheckFont(const Font * font) const
{
	LOG_THROW_IF(!font && defaultFontIdx == -1, "A default font must be specified before calling this method or a font must be specified as a parameter!");
}

void Plutonium::Menu::OnTextBoxGainFocus(const GuiItem * txt, EventArgs)
{
	LoseFocusExceptOne(txt);
}

void Plutonium::Menu::OnWindowClosed(const GUIWindow * wnd, EventArgs)
{
	if (wnd->ShouldHideUponClose())
	{
		const_cast<GUIWindow*>(wnd)->SetState(false);
		const_cast<GUIWindow*>(wnd)->Hide();
	}
	else MarkForDelete(wnd);
}