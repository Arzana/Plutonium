#pragma once
#include <tiny_obj_loader.h>

namespace Plutonium
{
	/* Defines the result of a tiyobj .pobj loader. */
	struct PobjLoaderResult
	{
		/* The attributes used by the spapes. */
		tinyobj::attrib_t Vertices;
		/* The shapes (mesh) defined by the model. */
		std::vector<tinyobj::gshape_t> Shapes;
		/* The portals defined in the model. */
		std::vector<tinyobj::portal_t> Portals;
		/* The material and texture parameters of the model. */
		std::vector<tinyobj::material_t> Materials;
		/* Whether the loading was successful. */
		bool Successful;
		/* Containserror and warning messages. */
		const char *Log;
	};

	/* Loads a .pobj file and it's associated .mtl file(s) (requires delete!). */
	_Check_return_ const PobjLoaderResult* _CrtLoadPobjMtl(_In_ const char *path);
}