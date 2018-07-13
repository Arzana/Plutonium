#include "Graphics\Materials\Material.h"
#include "Core\StringFunctions.h"

Plutonium::Material::Material(const char * name, AssetLoader * loader)
	: Name(heapstr(name)), loader(loader)
#if defined(DEBUG)
	, Debug(Color::Random(25))
#endif
{}

Plutonium::Material::~Material(void)
{
	free_s(Name);
}

void Plutonium::Material::ReleaseTexture(TextureHandler texture)
{
	if (!loader->Unload(texture)) delete_s(texture);
}