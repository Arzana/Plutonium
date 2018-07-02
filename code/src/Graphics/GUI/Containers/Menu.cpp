#include "Graphics\GUI\Containers\Menu.h"
#include "Core\StringFunctions.h"
#include "Core\Threading\PuThread.h"

using namespace Plutonium;

constexpr size_t MAX_STR_LEN = 64;

Plutonium::Menu::Menu(Game *game)
	: GameComponent(game), defaultFontIdx(-1),
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

GuiItem * Plutonium::Menu::GetControl(const char * name) const
{
	for (size_t i = 0; i < controlls.size(); i++)
	{
		GuiItem *cur = controlls.at(i);
		if (eqlstr(cur->GetName(), name)) return cur;
	}

	LOG_WAR("Unable to find control '%s'!", name);
	return nullptr;
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
	LOG("Initializing controll(%s).", _CRT_NAMEOF_RAW(GuiItem));

	GuiItem *result = new GuiItem(game);
	controlls.push_back(result);
	return result;
}

Label * Plutonium::Menu::AddLabel(const Font * font)
{
	LOG("Initializing controll(%s).", _CRT_NAMEOF_RAW(Label));
	CheckFont(font);

	Label *result = new Label(game, font ? font : loadedFonts.at(defaultFontIdx));
	controlls.push_back(result);
	return result;
}

Button * Plutonium::Menu::AddButton(const Font * font)
{
	LOG("Initializing controll(%s).", _CRT_NAMEOF_RAW(Button));
	CheckFont(font);

	Button *result = new Button(game, font ? font : loadedFonts.at(defaultFontIdx));
	controlls.push_back(result);
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

void Plutonium::Menu::DrawString(Vector2 position, const char * text, Color color)
{
#if defined (DEBUG)
	LOG_THROW_IF(defaultFontIdx == -1, "Unable to render debug text if no default font is specified!");

	const Font *font = loadedFonts.at(defaultFontIdx);
	const char32 *string = heapwstr(text);	// TODO: Fix memory leak.
	float lh = font->MeasureStringHeight(text);

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
	renderer->RenderTextForeground(position, 0.0f, color, font, string, stringVbo);
#endif
}

void Plutonium::Menu::Initialize(void)
{
	GameComponent::Initialize();
	Hide();
}

void Plutonium::Menu::Update(float dt)
{
	GameComponent::Update(dt);

	/* Call create once the loading is done. */
	if (callCreate)
	{
		callCreate = false;
		Create();
	}

	/* Update all GuiItems. */
	for (size_t i = 0; i < controlls.size(); i++)
	{
		controlls.at(i)->Update(dt);
	}
}

void Plutonium::Menu::Render(float dt)
{
	if (visible)
	{
		GameComponent::Render(dt);

		/* Render all GuiItems if the loading is complete. */
		for (size_t i = 0; i < controlls.size(); i++)
		{
			controlls.at(i)->Draw(renderer);
		}

		renderer->End();
	}
}

void Plutonium::Menu::Finalize(void)
{
	GameComponent::Finalize();

	for (size_t i = 0; i < controlls.size(); i++) delete_s(controlls.at(i));
	controlls.clear();

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