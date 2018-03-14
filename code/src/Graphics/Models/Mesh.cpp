#include "Graphics\Models\Mesh.h"
#include "Graphics\Models\ObjLoader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include <glad\glad.h>

using namespace Plutonium;
using namespace tinyobj;

Plutonium::Mesh::Mesh(const char * name)
	: Name(heapstr(name)), vertices(nullptr), vrtxCnt(0), buffer(nullptr)
{}

Plutonium::Mesh::~Mesh(void) noexcept
{
	free_cstr_s(Name);

	/* Release the GPU buffer if it has been created. */
	if (buffer) delete_s(buffer);
	/* Releases the CPU buffer if it still excist. */
	if (vertices) free_s(vertices);
}

void Plutonium::Mesh::Finalize(void)
{
	/* Make sure we don't allocate buffers twice. */
	LOG_THROW_IF(buffer, "Cannot call finalize a second time!");

	/* Create GPU buffer. */
	buffer = new Buffer();
	buffer->Bind(BindTarget::Array);
	buffer->SetData(BufferUsage::StaticDraw, vertices, vrtxCnt);

	/* Release CPU memory. */
	free_s(vertices);
	vertices = nullptr;
}

VertexFormat & Plutonium::Mesh::GetVertexAt(size_t idx) const
{
	/* Performs a range check on debug mode and checks if the buffers are still on the CPU. */
	ASSERT_IF(idx > vrtxCnt, "Attempting to retrieve vertex with out of bounds index!");
	ASSERT_IF(buffer, "Attempting to retrieve vertex after Finalize has been called!");

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