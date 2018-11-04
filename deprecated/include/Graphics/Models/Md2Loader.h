#pragma once
#include <vector>
#include <atomic>
#include "Core\Math\Constants.h"
#include "Core\Math\Vector2.h"
#include "Core\Math\Vector3.h"

namespace Plutonium
{
	/* Defines a simple 3 vertex face, defined by vertex indices and texture indices. */
	struct md2_triangle_t
	{
		/* The indices to the vertices of the triangle. */
		uint16 vertex_indices[3];
		/* The indices to the textures uv's of the triangle. */
		uint16 texture_indices[3];
	};

	/* Defines a simple 3D vertex. */
	struct md2_vertex_t
	{
		/* The position of the vertex. */
		Vector3 position;
		/* The normal at the position. */
		Vector3 normal;
	};

	/* Defines a animation frame. */
	struct md2_frame_t
	{
		/* The name of the frame. */
		const char *name;
		/* The scale that needs to be applied to all vertices in the frame. */
		Vector3 scale;
		/* The translation that needs to be applied to all vertices in the frame. */
		Vector3 translation;
		/* The vertices blob that the frame uses. */
		std::vector<md2_vertex_t> vertices;
	};

	/* Defines the result of an MD2 file load operation. */
	struct Md2LoaderResult
	{
		/* The OpenGL specific commands for this models. */
		std::vector<int32> OpenGLCommands;
		/* The required textures for the model (can be none). */
		std::vector<const char*> textures;
		/* The texture uv's at the model uses. */
		std::vector<Vector2> texcoords;
		/* All the faces that the model defines. */
		std::vector<md2_triangle_t> shapes;
		/* All animation frames of the model (can hold multiple animations). */
		std::vector<md2_frame_t> frames;

		/* Releases the resources allocated by the loader result. */
		~Md2LoaderResult(void);
	};

	/* Excecutes phase one of the MD2 loading and parsing process. */
	_Check_return_ const Md2LoaderResult* _CrtLoadMd2(_In_ const char *path, _In_opt_ std::atomic<float> *progression = nullptr, _In_opt_ float progressionMod = 1.0f);
}