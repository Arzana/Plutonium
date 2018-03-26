#pragma once
#include <tiny_obj_loader.h>

namespace Plutonium
{
	/* Defines the result of the tinyobj .obj loader. */
	struct ObjLoaderResult
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

		/* Initializes a new instance of a .obj loader result. */
		ObjLoaderResult(void)
			: Vertices(), Shapes(), Materials(),
			Successful(false), Log(nullptr)
		{}

		ObjLoaderResult(_In_ const ObjLoaderResult &value) = delete;
		ObjLoaderResult(_In_ ObjLoaderResult &&value) = delete;
		
		_Check_return_ ObjLoaderResult& operator =(_In_ const ObjLoaderResult &other) = delete;
		_Check_return_ ObjLoaderResult& operator =(_In_ ObjLoaderResult &&other) = delete;
	};

	/* Loads a .obj file and it's associated .mtl file(s) (requires delete!). */
	_Check_return_ const ObjLoaderResult* _CrtLoadObjMtl(_In_ const char *path);

	/* Gets the default material properties. */
	_Check_return_ tinyobj::material_t _CrtGetDefMtl(void);
}