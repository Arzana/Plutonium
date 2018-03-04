#pragma once
#include <vector>
#include "Core\Math\Constants.h"
#include "Core\Math\Vector2.h"
#include "Core\Math\Vector3.h"

namespace Plutonium
{
	struct md2_attrib_t
	{
		std::vector<Vector3> positions;
		std::vector<Vector2> texcoords;
		std::vector<Vector3> normals;
	};

	struct md2_triangle_t
	{
		std::vector<uint16> position_indices;
		std::vector<uint16> texture_indices;
	};

	struct md2_vertex_t
	{
		std::vector<byte> position_indices;
		byte normalIndex;
	};

	struct md2_frame_t
	{
		const char *Name;
		Vector3 Scale;
		Vector3 Translation;
		std::vector<md2_vertex_t> Vertices;
	};

	struct LoaderResult
	{
		int32 OpenGLCommandCount;

		std::vector<const char*> Textures;
		std::vector<md2_attrib_t> Vertices;
		std::vector<md2_triangle_t> Shapes;
		std::vector<md2_frame_t> Frames;
	};

	/* Loads a .md2 file (requires delete!). */
	_Check_return_ const LoaderResult* _CrtLoadMd2(_In_ const char *path);
}