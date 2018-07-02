#include "Graphics\PhongMaterial.h"
#include "Core\StringFunctions.h"
#include "Content\ObjLoader.h"
#include "Content\AssetLoader.h"

using namespace Plutonium;

Color randColor(void)
{
	static std::vector<Color> usedColors;

	Color result;
	do
	{
		result = Color(static_cast<uint32>((rand() % maxv<uint32>()) & Color::TransparentWhite.Packed));
	} while (std::find(usedColors.begin(), usedColors.end(), result) != usedColors.end());

	usedColors.push_back(result);
	return result;
}

Plutonium::PhongMaterial::PhongMaterial(void)
	: MaterialName(nullptr), Mesh(nullptr),
	AmbientMap(nullptr), DiffuseMap(nullptr), SpecularMap(nullptr), AlphaMap(nullptr), BumpMap(nullptr),
	Transmittance(Color::White), Ambient(Color::Black), Diffuse(Color::Black), Specular(Color::Black),
	SpecularExp(1.0f)
{}

Plutonium::PhongMaterial::PhongMaterial(Plutonium::Mesh * mesh, const ObjLoaderMaterial * material, AssetLoader *loader)
	: MaterialName(heapstr(material->Name)), loader(loader), Mesh(mesh),
	AmbientMap(nullptr), DiffuseMap(nullptr), SpecularMap(nullptr), AlphaMap(nullptr), BumpMap(nullptr),
	Transmittance(material->Transmittance), Ambient(material->Ambient), Diffuse(material->Diffuse), Specular(material->Specular),
	SpecularExp(material->HighlightExponent)
#if defined (DEBUG)
	, Debug(randColor())
#endif
{
	TextureCreationOptions opt;

	/* Check if ambient sampler is available. */
	if (strlen(material->AmbientMap.Path) > 0)
	{
		/* Set texture options and create sampler. */
		InitOptions(&material->AmbientMap, &opt);
		AmbientMap = loader->LoadTexture(material->AmbientMap.Path, false, &opt);
	}
	else AmbientMap = CreateDefault(loader->GetWindow(), "default ambient");

	/* Check if diffuse sampler is available. */
	if (strlen(material->DiffuseMap.Path) > 0)
	{
		/* Set texture options and create sampler. */
		InitOptions(&material->DiffuseMap, &opt);
		DiffuseMap = loader->LoadTexture(material->DiffuseMap.Path, false, &opt);
	}
	else DiffuseMap = CreateDefault(loader->GetWindow(), "default diffuse");

	/* Check if specular sampler is available. */
	if (strlen(material->SpecularMap.Path) > 0)
	{
		/* Set texture options and create sampler. */
		InitOptions(&material->SpecularMap, &opt);
		SpecularMap = loader->LoadTexture(material->SpecularMap.Path, false, &opt);
	}
	else SpecularMap = CreateDefault(loader->GetWindow(), "default specular");

	/* Check if alpha sampler is available. */
	if (strlen(material->AlphaMap.Path) > 0)
	{
		/* Set textyre options and create sampler. */
		InitOptions(&material->AlphaMap, &opt);
		AlphaMap = loader->LoadTexture(material->AlphaMap.Path, false, &opt);
	}
	else AlphaMap = CreateDefault(loader->GetWindow(), "default alpha");

	/* Check if bump sampler is available. */
	if (strlen(material->BumpMap.Path) > 0)
	{
		InitOptions(&material->BumpMap, &opt);
		BumpMap = loader->LoadTexture(material->BumpMap.Path, false, &opt);
	}
	else BumpMap = CreateDefault(loader->GetWindow(), "default bump", Color::Malibu);
}

Plutonium::PhongMaterial::~PhongMaterial(void) noexcept
{
	free_s(MaterialName);
	delete_s(Mesh);
	if (!loader->Unload(AmbientMap)) delete_s(AmbientMap);
	if (!loader->Unload(DiffuseMap)) delete_s(DiffuseMap);
	if (!loader->Unload(SpecularMap)) delete_s(SpecularMap);
	if (!loader->Unload(AlphaMap)) delete_s(AlphaMap);
	if (!loader->Unload(BumpMap)) delete_s(BumpMap);
}

Plutonium::PhongMaterial * Plutonium::PhongMaterial::GetDefault(void)
{
	/* Initialize variables. */
	PhongMaterial *result = new PhongMaterial();
	WindowHandler wnd = result->loader->GetWindow();

	/* Set neutral maps. */
	result->AmbientMap = CreateDefault(wnd, "default ambient");
	result->DiffuseMap = CreateDefault(wnd, "default diffuse");
	result->SpecularMap = CreateDefault(wnd, "default specular");
	result->AlphaMap = CreateDefault(wnd, "default alpha");
	result->BumpMap = CreateDefault(wnd, "default bump", Color::Malibu);

	return result;
}

void Plutonium::PhongMaterial::InitOptions(const ObjLoaderTextureMap * objOpt, TextureCreationOptions * texOpt)
{
	texOpt->SetWrapping(objOpt->ClampedCoords ? WrapMode::ClampToEdge : WrapMode::Repeat);
	texOpt->Gain = objOpt->Brightness;
	texOpt->Range = objOpt->Contrast;
}

Plutonium::Texture * Plutonium::PhongMaterial::CreateDefault(WindowHandler wnd, const char *name, Color filler)
{
	Texture *result = new Texture(1, 1, wnd, &TextureCreationOptions::DefaultNoMipMap, name);
	result->SetData(filler.ToArray());
	return result;
}