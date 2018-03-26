#define TINYOBJLOADER_IMPLEMENTATION

#include "Graphics\Models\ObjLoader.h"
#include "Core\Diagnostics\Logging.h"
#include "Streams\FileReader.h"

using namespace Plutonium;
using namespace tinyobj;
using namespace std;

const ObjLoaderResult * Plutonium::_CrtLoadObjMtl(const char * path)
{
	/* Create new result structure. */
	ObjLoaderResult *result = new ObjLoaderResult();
	FileReader reader(path, true);

	/* Load .obj and .mtl files. */
	string err;
	result->Successful = LoadObj(&result->Vertices, &result->Shapes, &result->Materials, &err, path, reader.GetFileDirectory());
	LOG_WAR_IF(result->Successful && !err.empty(), "TinyObj loader warnings:\n%s", err.c_str());

	/* Convert error to log and return result. */
	result->Log = err.c_str();
	return result;
}

tinyobj::material_t Plutonium::_CrtGetDefMtl(void)
{
	return tinyobj::material_t();
}
