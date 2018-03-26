#define TINYOBJLOADER_IMPLEMENTATION

#include "Graphics\Portals\PobjLoader.h"
#include "Core\Diagnostics\Logging.h"
#include "Streams\FileReader.h"

using namespace Plutonium;
using namespace tinyobj;
using namespace std;

const PobjLoaderResult * Plutonium::_CrtLoadPobjMtl(const char * path)
{
	/* Create new result structure. */
	PobjLoaderResult *result = new PobjLoaderResult();
	FileReader reader(path, true);

	/* Load .pobj and .mtl files. */
	string err;
	result->Successful = LoadPobj(&result->Vertices, &result->Shapes, &result->Portals, &result->Materials, &err, path, reader.GetFileDirectory());
	LOG_WAR_IF(result->Successful && !err.empty(), "TinyObj loader warnings:\n%s", err.c_str());

	/* Convert error to lod and return result. */
	result->Log = err.c_str();
	return result;
}