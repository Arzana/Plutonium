#include "Graphics\Models\Mesh.h"
#include "Graphics\Models\ObjLoader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include <glad\glad.h>

using namespace Plutonium;
using namespace tinyobj;

Plutonium::Mesh::Mesh(const char * name)
	: Name(heapstr(name)), vertices(nullptr), vrtxCnt(0), ptr(0)
{}

Plutonium::Mesh::~Mesh(void) noexcept
{
	free_cstr_s(Name);

	/* Releases the buffers if they are created. */
	if (ptr != 0) glDeleteBuffers(1, &ptr);

	/* Releases the CPU buffers if they still excist. */
	if (vertices) free_s(vertices);
}

void Plutonium::Mesh::Finalize(void)
{
	/* Make sure we don't allocate buffers twice. */
	LOG_THROW_IF(ptr != 0, "Cannot call Finalize a second time!");

	/* Allocated vertex and index buffers on the GPU. */
	glGenBuffers(1, &ptr);

	/* Copy vertices to the GPU. */
	glBindBuffer(GL_ARRAY_BUFFER, ptr);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * vrtxCnt, void_ptr(vertices), GL_STATIC_DRAW);

	/* Release CPU memory. */
	free_s(vertices);
	vertices = nullptr;
}

uint32 Plutonium::Mesh::GetVertexBuffer(void) const
{
	/* On debug check if the buffer has been created. */
#if defined(DEBUG)
	LOG_THROW_IF(ptr == 0, "Attempting to get vertex buffer before Finalize is called!");
#endif

	return ptr;
}

VertexFormat & Plutonium::Mesh::GetVertexAt(size_t idx) const
{
	/* Performs a range check on debug mode and checks if the buffers are still on the CPU. */
#if defined(DEBUG)
	LOG_THROW_IF(idx > vrtxCnt, "Attempting to retrieve vertex with out of bounds index!");
	LOG_THROW_IF(ptr == 0, "Attempting to retrieve vertex after Finalize has been called!");
#endif

	return vertices[idx];
}

Mesh * Plutonium::Mesh::FromFile(const LoaderResult * buffer, size_t idx)
{
	/* Get current shape. */
	shape_t shape = buffer->Shapes.at(idx);

	/* Define result. */
	Mesh *result = new Mesh(shape.name.c_str());

	/* Allocate buffers. */
	for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); i++) result->vrtxCnt += shape.mesh.num_face_vertices.at(i);
	result->vertices = malloc_s(VertexFormat, result->vrtxCnt);

	/* Copy vertices to buffer. */
	for (size_t i = 0, start = 0; i < shape.mesh.num_face_vertices.size(); i++)
	{
		size_t verticesInFace = shape.mesh.num_face_vertices.at(i);

		/* Loop through vertices in current face. */
		for (size_t j = 0; j < verticesInFace; j++)
		{
			size_t k = start + j;
			index_t idx = shape.mesh.indices.at(k);

			/* Copy over current vertex. */
			VertexFormat format;
			format.Position.X = buffer->Vertices.vertices.at(3 * idx.vertex_index);
			format.Position.Y = buffer->Vertices.vertices.at(3 * idx.vertex_index + 1);
			format.Position.Z = buffer->Vertices.vertices.at(3 * idx.vertex_index + 2);
			format.Normal.X = buffer->Vertices.normals.at(3 * idx.normal_index);
			format.Normal.Y = buffer->Vertices.normals.at(3 * idx.normal_index + 1);
			format.Texture.X = buffer->Vertices.texcoords.at(2 * idx.texcoord_index);
			format.Texture.Y = buffer->Vertices.texcoords.at(2 * idx.texcoord_index + 1);

			/* Push vertex and index to buffer  */
			result->vertices[k] = format;
		}

		/* Add the offset to the start. */
		start += verticesInFace;
	}

	return result;
}