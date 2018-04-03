#include "Graphics\Portals\PobjLoader.h"
#include "Core\Diagnostics\Logging.h"
#include "Streams\FileReader.h"

using namespace Plutonium;
using namespace tinyobj;
using namespace std;

/*
The .pobj file specifies some extra tokens within a normal .obj file.
- Room name (r <string>):										This fimply specifies the name of the following euclid room.
- Gravity (vg <normal index):									This defines the gravity direction within the specified room.
- Portal (p <length> <vertex indices...> <destination index>):	This defines a portal, starting with the frame mesh and ending with the destination room index.
*/

const PobjLoaderResult * Plutonium::_CrtLoadPobjMtl(const char * path)
{
	/* Create new result structure. */
	PobjLoaderResult *result = new PobjLoaderResult();
	FileReader reader(path, true);

	/* Load .pobj and .mtl files. */
	string err;
	result->Successful = LoadPobj(&result->Vertices, &result->Rooms, &result->Materials, &err, path, reader.GetFileDirectory());
	LOG_WAR_IF(result->Successful && !err.empty(), "TinyObj loader warnings:\n%s", err.c_str());

	/* Convert error to lod and return result. */
	result->Log = err.c_str();
	return result;
}