#include "Graphics\Materials\MaterialBP.h"
#include "Core\StringFunctions.h"
#include <string>

Plutonium::MaterialBP::MaterialBP(const char * name, AssetLoader * loader)
	: Material(name, loader), Ambient(nullptr), Diffuse(nullptr),
	Specular(nullptr), Opacity(nullptr), Normal(nullptr),
	SpecularExponent(0.0f)
{}

Plutonium::MaterialBP::MaterialBP(const char * name, AssetLoader * loader, Texture * ambient, Texture * diffuse, Texture * specular, Texture * opacity, Texture * normal, float specularExponent)
	: MaterialBP(name, loader)
{
	SpecularExponent = specularExponent;

	Ambient = ambient ? ambient : CreateDefault("DefaultAmbient", Color::Black);
	Diffuse = diffuse ? diffuse : CreateDefault("DefaultDiffuse", Color::Black);
	Specular = specular ? specular : CreateDefault("DefaultSpecular", Color::Black);
	Opacity = opacity ? opacity : CreateDefault("DefaultOpacity", Color::White);
	Normal = normal ? normal : CreateDefault("DefaultNormal", Color::Malibu);
}

Plutonium::MaterialBP::MaterialBP(const ObjLoaderMaterial * blueprint, AssetLoader * loader)
	: MaterialBP(blueprint->Name, loader)
{
	TextureCreationOptions config;

	/* Set specular exponent. */
	SpecularExponent = blueprint->HighlightExponent;

	/* Import ambient map is needed. */
	if (!nullorempty(blueprint->AmbientMap.Path))
	{
		InitConfig(&blueprint->AmbientMap, blueprint->Ambient, &config);
		Ambient = loader->LoadTexture(blueprint->AmbientMap.Path, false, &config);
	}
	else Ambient = CreateDefault("DefaultAmbient", Color::Black);

	/* Import diffuse map is needed. */
	if (!nullorempty(blueprint->DiffuseMap.Path))
	{
		InitConfig(&blueprint->DiffuseMap, blueprint->Diffuse, &config);
		Diffuse = loader->LoadTexture(blueprint->DiffuseMap.Path, false, &config);
	}
	else Diffuse = CreateDefault("DefaultDiffuse", Color::Black);

	/* Import specular map is needed. */
	if (!nullorempty(blueprint->SpecularMap.Path))
	{
		InitConfig(&blueprint->SpecularMap, blueprint->Specular, &config);
		Specular = loader->LoadTexture(blueprint->SpecularMap.Path, false, &config);
	}
	else Specular = CreateDefault("DefaultSpecular", Color::Black);

	/* Import opacity map is needed. */
	if (!nullorempty(blueprint->AlphaMap.Path))
	{
		InitConfig(&blueprint->AlphaMap, Color::White, &config);
		Opacity = loader->LoadTexture(blueprint->AlphaMap.Path, false, &config);
	}
	else Opacity = CreateDefault("DefaultOpacity", Color::White);

	/* Import normal map is needed. */
	if (!nullorempty(blueprint->BumpMap.Path))
	{
		InitConfig(&blueprint->BumpMap, Color::White, &config);
		Normal = loader->LoadTexture(blueprint->BumpMap.Path, false, &config);
	}
	else Normal = CreateDefault("DefaultNormal", Color::Malibu);
}

Plutonium::MaterialBP::~MaterialBP(void)
{
	ReleaseTexture(Ambient);
	ReleaseTexture(Diffuse);
	ReleaseTexture(Specular);
	ReleaseTexture(Opacity);
	ReleaseTexture(Normal);
}

void Plutonium::MaterialBP::InitConfig(const ObjLoaderTextureMap * texture, Color filter, TextureCreationOptions * output)
{
	output->SetWrapping(texture->ClampedCoords ? WrapMode::ClampToEdge : WrapMode::Repeat);
	output->Gain = texture->Brightness;
	output->Range = texture->Contrast;
	output->Filter = filter;
}

Plutonium::Texture * Plutonium::MaterialBP::CreateDefault(const char * name, Color filler)
{
	Texture *result = new Texture(1, 1, loader->GetWindow(), &TextureCreationOptions::DefaultNoMipMap, name);
	result->SetData(filler.ToArray());
	return result;
}