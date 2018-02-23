#pragma once
#include <tiny_obj_loader.h>

namespace Plutonium
{
	/* Defines the result of the tinyobj .obj loader. */
	struct LoaderResult
	{
		/* The attributes used by the shapes. */
		tinyobj::attrib_t Vertices;
		/* The shapes (mesh) defined by the model. */
		std::vector<tinyobj::shape_t> Shapes;
		/* The material and texture parameters of the model. */
		std::vector<tinyobj::material_t> Materials;
		/* Whether the loading was successful. */
		bool Successful;
		/* Contains error and warning messages. */
		const char *Log;
	};

	/* Loads a .obj file and it's associated .mtl files (requires delete!). */
	_Check_return_ const LoaderResult* _CrtLoadObjMtl(_In_ const char *path);

	/* Gets the default material properties. */
	_Check_return_ tinyobj::material_t _CrtGetDefMtl(void);
}