#pragma once
#include <tiny_obj_loader.h>

namespace Plutonium
{
	/* Defines the result of a tiyobj .pobj loader. */
	struct PobjLoaderResult
	{
		/* The attributes used by the spapes. */
		tinyobj::attrib_t Vertices;
		/* The rooms defined by the model. */
		std::vector<tinyobj::room_t> Rooms;
		/* The material and texture parameters of the model. */
		std::vector<tinyobj::material_t> Materials;
		/* Whether the loading was successful. */
		bool Successful;
		/* Containserror and warning messages. */
		const char *Log;

		/* Initializes a new instance of a .pobj loader result. */
		PobjLoaderResult(void)
			: Vertices(), Rooms(), Materials(),
			Successful(false), Log(nullptr)
		{}

		PobjLoaderResult(_In_ const PobjLoaderResult &value) = delete;
		PobjLoaderResult(_In_ PobjLoaderResult &&value) = delete;

		_Check_return_ PobjLoaderResult& operator =(_In_ const PobjLoaderResult &other) = delete;
		_Check_return_ PobjLoaderResult& operator =(_In_ PobjLoaderResult &&other) = delete;
	};

	/* Loads a .pobj file and it's associated .mtl file(s) (requires delete!). */
	_Check_return_ const PobjLoaderResult* _CrtLoadPobjMtl(_In_ const char *path);
}