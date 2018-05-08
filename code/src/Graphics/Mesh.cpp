#include "Graphics\Mesh.h"
#include "Content\ObjLoader.h"
#include "Graphics\Portals\PobjLoader.h"
#include "Graphics\Models\Md2Loader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include <glad\glad.h>

using namespace Plutonium;
using namespace tinyobj;

Plutonium::Mesh::Mesh(const char * name)
	: Name(heapstr(name)), vertices(nullptr), vrtxCnt(0), buffer(nullptr)
{
	LOG("Creating mesh '%s'.", name);
}

Plutonium::Mesh::~Mesh(void) noexcept
{
	free_s(Name);

	/* Release the GPU buffer if it has been created. */
	if (buffer) delete_s(buffer);
	/* Releases the CPU buffer if it still excist. */
	if (vertices) free_s(vertices);
}

void Plutonium::Mesh::Finalize(WindowHandler wnd)
{
	/* Make sure we don't allocate buffers twice. */
	LOG_THROW_IF(buffer, "Cannot call finalize a second time!");

	/* Create GPU buffer. */
	buffer = new Buffer(wnd, BindTarget::Array);
	buffer->SetData(BufferUsage::StaticDraw, vertices, vrtxCnt);

	/* Release CPU memory. */
	free_s(vertices);
	vertices = nullptr;
}

void Plutonium::Mesh::SetBufferSize(size_t size)
{
	/* Check if mesh if still valid. */
	ASSERT_IF(buffer, "Attempting to buffer vertex after Finalize has been called!");

	/* Check if buffer size needs to increase. */
	if (size > vrtxCnt)
	{
		/* Increase buffer. */
		if (!vrtxCnt) vertices = malloc_s(VertexFormat, size);
		else realloc_s(VertexFormat, vertices, size);

		/* Set new size. */
		vrtxCnt = size;
	}
}

VertexFormat & Plutonium::Mesh::GetVertexAt(size_t idx) const
{
	/* Performs a range check on debug mode and checks if the buffers are still on the CPU. */
	ASSERT_IF(idx > vrtxCnt, "Attempting to retrieve vertex with out of bounds index!");
	ASSERT_IF(buffer, "Attempting to retrieve vertex after Finalize has been called!");

	return vertices[idx];
}

void Plutonium::Mesh::Append(Mesh * other)
{
	/* Check for correct use. */
	ASSERT_IF(buffer, "Attempting to append to finalized mesh!");
	ASSERT_IF(other->buffer, "Cannot append finalized mesh into mesh!");

	/* Ensure buffer size. */
	size_t start = vrtxCnt;
	SetBufferSize(vrtxCnt + other->vrtxCnt);

	/* Copy over vertices. */
	for (size_t i = 0, j = start; i < other->vrtxCnt; i++, j++) vertices[j] = other->vertices[i];
}

Mesh * Plutonium::Mesh::FromFile(const ObjLoaderResult * buffer, size_t idx)
{
	/* Get desired shape. */
	const ObjLoaderMesh &mesh = buffer->Shapes.at(idx);

	/* Create result and allocate memory. */
	Mesh *result = new Mesh(mesh.Name);
	result->vrtxCnt = mesh.Indices.size();
	result->vertices = malloc_s(VertexFormat, result->vrtxCnt);

	/* Copy vertices. */
	for (size_t i = 0; i < mesh.Indices.size(); i++)
	{
		const ObjLoaderVertex &idx = mesh.Indices.at(i);

		result->vertices[i].Position = buffer->Vertices.at(idx.Vertex);
		result->vertices[i].Normal = buffer->Normals.at(idx.Normal);
		result->vertices[i].Texture = buffer->TexCoords.at(idx.TexCoord);
	}

	return result;
}

Mesh * Plutonium::Mesh::RFromFile(const PobjLoaderResult * buffer, size_t ridx, size_t sidx)
{
	/* Get current shape. */
	shape_t shape = buffer->Rooms.at(ridx).shapes.at(sidx);

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
			format.Normal.Z = buffer->Vertices.normals.at(3 * idx.normal_index + 2);
			format.Texture.X = buffer->Vertices.texcoords.at(2 * idx.texcoord_index);
			format.Texture.Y = buffer->Vertices.texcoords.at(2 * idx.texcoord_index + 1);

			/* Push vertex to buffer  */
			result->vertices[k] = format;
		}

		/* Add the offset to the start. */
		start += verticesInFace;
	}

	return result;
}

Mesh * Plutonium::Mesh::PFromFile(const PobjLoaderResult * buffer, size_t ridx, size_t pidx)
{
	/* Get current portal shape. */
	portal_t portal = buffer->Rooms.at(ridx).portals.at(pidx);

	/* Define result. */
	Mesh *result = new Mesh("Portal");

	/* Allocate buffers. */
	result->vrtxCnt = portal.vertices.size();
	result->vertices = malloc_s(VertexFormat, result->vrtxCnt);

	/* Copy vertices to buffer. */
	for (size_t i = 0; i < portal.vertices.size(); i++)
	{
		int idx = portal.vertices.at(i);

		/* Copy over current vertex. */
		VertexFormat format;
		format.Position.X = buffer->Vertices.vertices.at(3 * idx);
		format.Position.Y = buffer->Vertices.vertices.at(3 * idx + 1);
		format.Position.Z = buffer->Vertices.vertices.at(3 * idx + 2);
		format.Normal = Vector3::Zero;
		format.Texture = Vector2::Zero;

		/* Push vertex to buffer. */
		result->vertices[i] = format;
	}

	return result;
}

Mesh * Plutonium::Mesh::FromFile(const Md2LoaderResult * buffer, size_t idx)
{
	/* Get specified frame. */
	const md2_frame_t& frame = buffer->frames.at(idx);

	/* Create result. */
	Mesh *result = new Mesh(frame.name);
	result->vrtxCnt = buffer->shapes.size() * 3;
	result->vertices = malloc_s(VertexFormat, result->vrtxCnt);

	/* Loop through all triangles. */
	for (size_t i = 0, k = 0; i < buffer->shapes.size(); i++)
	{
		/* Loop through all vertices in triangle. */
		const md2_triangle_t &trgl = buffer->shapes.at(i);
		for (size_t j = 0; j < 3; j++)
		{
			/* Get current vertex. */
			md2_vertex_t vrtx = frame.vertices.at(trgl.vertex_indices[j]);

			/* Copy over current vertex. */
			size_t test = k + j;
			result->vertices[test].Position = frame.scale * vrtx.position + frame.translation;
			result->vertices[test].Normal = Vector3(vrtx.normal.X, vrtx.normal.Y, vrtx.normal.Z);
			result->vertices[test].Texture = buffer->texcoords.at(trgl.texture_indices[j]);
		}

		k += 3;
	}

	return result;
}