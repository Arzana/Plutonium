#include "Graphics\Models\ObjLoader.h"

using namespace Plutonium;
using namespace tinyobj;
using namespace std;

const LoaderResult * Plutonium::_CrtLoadObjMtl(const char * path)
{
	/* Create new result structure. */
	LoaderResult *result = new LoaderResult();

	/* Load .obj and .mtl files. */
	string err;
	result->Successful = LoadObj(&result->Vertices, &result->Shapes, &result->Materials, &err, path);

	/* Convert error to log and result result. */
	result->Log = err.c_str();
	return result;
}
