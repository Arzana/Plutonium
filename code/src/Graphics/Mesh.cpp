#include "Graphics\Mesh.h"
#include "Content\ObjLoader.h"
#include "Graphics\Models\Md2Loader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include <glad\glad.h>

using namespace Plutonium;

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

VertexFormat * Plutonium::Mesh::GetVertexAt(size_t idx) const
{
	/* Performs a range check on debug mode and checks if the buffers are still on the CPU. */
	ASSERT_IF(idx > vrtxCnt, "Attempting to retrieve vertex with out of bounds index!");
	ASSERT_IF(buffer, "Attempting to retrieve vertex after Finalize has been called!");

	return vertices + idx ;
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

#include "Core\String.h"

void Plutonium::Mesh::SetTangent(VertexFormat & vrtx1, VertexFormat & vrtx2, VertexFormat & vrtx3)
{
	/* Calculate the edges and delta uv's. */
	Vector3 e1 = vrtx2.Position - vrtx1.Position;
	Vector3 e2 = vrtx3.Position - vrtx1.Position;
	Vector2 dUv1 = vrtx2.Texture - vrtx1.Texture;
	Vector2 dUv2 = vrtx3.Texture - vrtx1.Texture;

	/* Calculate tangent and bitangent. */
	Vector3 tangent = normalize((dUv2.Y * e1 - dUv1.Y * e2) / prepdot(dUv1, dUv2));

	/* Set tangent. */
	vrtx1.Tangent = tangent;
	vrtx2.Tangent = tangent;
	vrtx3.Tangent = tangent;
}

void Plutonium::Mesh::SetNormal(VertexFormat & vrtx1, VertexFormat & vrtx2, VertexFormat & vrtx3)
{
	/* Calculate a surface normal and normalize it in the same place in memory. */
	Vector3 n = cross(vrtx2.Position - vrtx3.Position, vrtx1.Position - vrtx2.Position);
	n.Normalize();

	/* Assign normal to all vertexes. */
	vrtx1.Normal = n;
	vrtx2.Normal = n;
	vrtx3.Normal = n;
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
	bool redefineNormal = false;
	for (size_t i = 0, j = 0; i < mesh.Indices.size(); i++, j++)
	{
		const ObjLoaderVertex &idx = mesh.Indices.at(i);

		/* Copy over the position and the texture coordinate. */
		result->vertices[i].Position = buffer->Vertices.at(idx.Vertex);
		result->vertices[i].Texture = buffer->TexCoords.at(idx.TexCoord);

		/* If a normal is present just copy it over otherwise specify that the normal needs to be redefined for this triangle. */
		if (idx.Normal != -1) result->vertices[i].Normal = buffer->Normals.at(idx.Normal);
		else redefineNormal = true;

		/* Set tangent (and normal if needed) for the last three vertices. */
		if (j > 1)
		{
			VertexFormat &vrtx1 = result->vertices[i - 2];
			VertexFormat &vrtx2 = result->vertices[i - 1];
			VertexFormat &vrtx3 = result->vertices[i];

			if (redefineNormal)
			{
				SetNormal(vrtx1, vrtx2, vrtx3);
				redefineNormal = false;
			}

			j = -1;
			SetTangent(vrtx1, vrtx2, vrtx3);
		}
	}

	return result;
}

Mesh * Plutonium::Mesh::FromFile(const Md2LoaderResult * buffer, size_t idx)
{
	/* Get desired shape. */
	const md2_frame_t &frame = buffer->frames.at(idx);

	/* Create result and allocate memory. */
	Mesh *result = new Mesh(frame.name);
	result->vrtxCnt = buffer->shapes.size() * 3;
	result->vertices = malloc_s(VertexFormat, result->vrtxCnt);

	/* Copy vertices. */
	for (size_t i = 0, k = 0; i < buffer->shapes.size(); i++)
	{
		const md2_triangle_t &trgl = buffer->shapes.at(i);
		for (size_t j = 0; j < 3; j++, k++)
		{
			const md2_vertex_t vrtx = frame.vertices.at(trgl.vertex_indices[j]);

			/* Apply scale and translation per frame. */
			result->vertices[k].Position = frame.scale * vrtx.position + frame.translation;
			result->vertices[k].Normal = vrtx.normal;
			result->vertices[k].Texture = buffer->texcoords.at(trgl.texture_indices[j]);
		}

		/* Set tangent for the last three vertices. */
		SetTangent(result->vertices[k - 3], result->vertices[k - 2], result->vertices[k - 1]);
	}

	return result;
}