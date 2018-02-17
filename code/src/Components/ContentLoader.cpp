#include "Components\ContentLoader.h"
#include "Core\StringFunctions.h"
#include <cstring>

Plutonium::ContentLoader::ContentLoader(void)
	: root(""), rootLen(0)
{}

void Plutonium::ContentLoader::SetRoot(const char * directory)
{
	/* Set new root and calculate length or directory to save on loading time later when creating the path buffer. */
	root = directory;
	rootLen = strlen(root);
}
