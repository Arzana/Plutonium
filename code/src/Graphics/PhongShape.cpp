#include "Graphics\PhongShape.h"
#include "Core\StringFunctions.h"
#include "Content\ObjLoader.h"

Plutonium::PhongShape::PhongShape(void)
	: MaterialName(nullptr), Mesh(nullptr),
	AmbientMap(nullptr), DiffuseMap(nullptr), SpecularMap(nullptr), AlphaMap(nullptr),
	Transmittance(Color::White), Ambient(Color::Black), Diffuse(Color::Black), Specular(Color::Black),
	SpecularExp(1.0f)
{}

Plutonium::PhongShape::PhongShape(Plutonium::Mesh * mesh, const ObjLoaderMaterial * material)
	: MaterialName(heapstr(material->Name)), Mesh(mesh),
	AmbientMap(nullptr), DiffuseMap(nullptr), SpecularMap(nullptr), AlphaMap(nullptr),
	Transmittance(material->Transmittance), Ambient(material->Ambient), Diffuse(material->Diffuse), Specular(material->Specular),
	SpecularExp(material->HighlightExponent)
{
	TextureCreationOptions opt;

	/* Check if ambient sampler is available. */
	if (strlen(material->AmbientMap.Path) > 0)
	{
		/* Set texture options and create sampler. */
		InitOptions(&material->AmbientMap, &opt);
		AmbientMap = Texture::FromFile(material->AmbientMap.Path, &opt);
	}
	else AmbientMap = CreateDefault();

	/* Check if diffuse sampler is available. */
	if (strlen(material->DiffuseMap.Path) > 0)
	{
		/* Set texture options and create sampler. */
		InitOptions(&material->DiffuseMap, &opt);
		DiffuseMap = Texture::FromFile(material->DiffuseMap.Path, &opt);
	}
	else DiffuseMap = CreateDefault();

	/* Check if specular sampler is available. */
	if (strlen(material->SpecularMap.Path) > 0)
	{
		/* Set texture options and create sampler. */
		InitOptions(&material->SpecularMap, &opt);
		SpecularMap = Texture::FromFile(material->SpecularMap.Path, &opt);
	}
	else SpecularMap = CreateDefault();

	/* Check if alpha sampler is available. */
	if (strlen(material->AlphaMap.Path) > 0)
	{
		/* Set textyre options and create sampler. */
		InitOptions(&material->AlphaMap, &opt);
		AlphaMap = Texture::FromFile(material->AlphaMap.Path, &opt);
	}
	else AlphaMap = CreateDefault();
}

Plutonium::PhongShape::~PhongShape(void) noexcept
{
	free_s(MaterialName);
	delete_s(Mesh);
	if (AmbientMap) delete_s(AmbientMap);
	if (DiffuseMap) delete_s(DiffuseMap);
	if (SpecularMap) delete_s(SpecularMap);
	if (AlphaMap) delete_s(AlphaMap);
}

void Plutonium::PhongShape::InitOptions(const ObjLoaderTextureMap * objOpt, TextureCreationOptions * texOpt)
{
	texOpt->SetWrapping(objOpt->ClampedCoords ? WrapMode::ClampToEdge : WrapMode::Repeat);
	texOpt->Gain = objOpt->Brightness;
	texOpt->Range = objOpt->Contrast;
}

Plutonium::Texture * Plutonium::PhongShape::CreateDefault(void)
{
	static Color data[] = { Color::White, Color::White, Color::White, Color::White };

	Texture *result = new Texture(2, 2, 0, "default");
	result->SetData(reinterpret_cast<byte*>(data));
	return result;
}