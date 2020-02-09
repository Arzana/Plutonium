#include "TangentGenerator.h"
#include <Streams/BinaryReader.h>
#include <Core/Diagnostics/Logging.h>
#include <mikktspace/mikktspace.h>

using namespace Pu;

/* We need both the full model and the mesh to figure out the tangents. */
struct MikkTSpaceUserData
{
	const PumIntermediate &Data;
	pum_mesh &Mesh;
	Vector4 *Tangents;
	size_t stride;

	MikkTSpaceUserData(const PumIntermediate &data, pum_mesh &mesh)
		: Data(data), Mesh(mesh), stride(mesh.GetStride())
	{
		Tangents = reinterpret_cast<Vector4*>(malloc(sizeof(Vector4) * (Mesh.VertexViewSize / stride)));
	}

	~MikkTSpaceUserData(void)
	{
		free(Tangents);
	}

	inline size_t GetIdx(int faceIdx, int vertIdx) const
	{
		return (faceIdx * stride * 3) + (vertIdx * stride);
	}
};

int PumGetNumFaces(const SMikkTSpaceContext *context)
{
	/* The amount of faces are always specified in the vertex view, never in the index view. */
	const MikkTSpaceUserData &data = *reinterpret_cast<MikkTSpaceUserData*>(context->m_pUserData);
	return static_cast<int>((data.Mesh.VertexViewSize / data.stride) / 3);
}

int PumGetNumVerticesOfFace(const SMikkTSpaceContext *context, const int)
{
	/* PuM only allows for triangles. */
	return 3;
}

void PumGetPosition(const SMikkTSpaceContext *context, float posOut[3], const int faceIdx, const int vertIdx)
{
	/* A binary reader is quite cheap to create so we just create new ones every time. */
	const MikkTSpaceUserData &data = *reinterpret_cast<MikkTSpaceUserData*>(context->m_pUserData);
	BinaryReader reader{ data.Data.Data.GetData() + data.Mesh.VertexViewStart, data.Mesh.VertexViewSize, Endian::Little };

	/* This vertex will be stored in the vertex view, at the face index (3 vertices) plus the vertex index. */
	const size_t offset = data.GetIdx(faceIdx, vertIdx);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(offset));
	const Vector3 pos = reader.PeekVector3();

	/* Just copy over the vector. */
	posOut[0] = pos.X;
	posOut[1] = pos.Y;
	posOut[2] = pos.Z;
}

void PumGetNormal(const SMikkTSpaceContext *context, float normalOut[3], const int faceIdx, const int vertIdx)
{
	/* A binary reader is quite cheap to create so we just create new ones every time. */
	const MikkTSpaceUserData &data = *reinterpret_cast<MikkTSpaceUserData*>(context->m_pUserData);
	BinaryReader reader{ data.Data.Data.GetData() + data.Mesh.VertexViewStart, data.Mesh.VertexViewSize, Endian::Little };

	/* This vertex will be stored in the vertex view, at the face index (3 vertices) plus the vertex index, and we skip the position. */
	const size_t offset = data.GetIdx(faceIdx, vertIdx) + sizeof(Vector3);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(offset));
	const Vector3 normal = reader.PeekVector3();

	/* Just copy over the vector. */
	normalOut[0] = normal.X;
	normalOut[1] = normal.Y;
	normalOut[2] = normal.Z;
}

void PumGetTexCoord(const SMikkTSpaceContext *context, float coordOut[2], const int faceIdx, const int vertIdx)
{
	/* A binary reader is quite cheap to create so we just create new ones every time. */
	const MikkTSpaceUserData &data = *reinterpret_cast<MikkTSpaceUserData*>(context->m_pUserData);
	BinaryReader reader{ data.Data.Data.GetData() + data.Mesh.VertexViewStart, data.Mesh.VertexViewSize, Endian::Little };

	/* This vertex will be stored in the vertex view, at the face index (3 vertices) plus the vertex index, and we skip the position and normal. */
	const size_t offset = data.GetIdx(faceIdx, vertIdx) + (sizeof(Vector3) << 1);
	reader.Seek(SeekOrigin::Begin, static_cast<int64>(offset));
	const Vector2 coord = reader.PeekVector2();

	/* Just copy over the vector. */
	coordOut[0] = coord.X;
	coordOut[1] = coord.Y;
}

void PumSetTangent(const SMikkTSpaceContext *context, const float tangent[3], const float sign, const int faceIdx, const int vertIdx)
{
	MikkTSpaceUserData &data = *reinterpret_cast<MikkTSpaceUserData*>(context->m_pUserData);
	const size_t offset = faceIdx * 3 + vertIdx;
	data.Tangents[offset] = Vector4(tangent[0], tangent[1], tangent[2], sign);
}

void GenerateTangents(PumIntermediate & data)
{
	/* Set all of the delegates. */
	SMikkTSpaceInterface interfaces{};
	interfaces.m_getNumFaces = &PumGetNumFaces;
	interfaces.m_getNumVerticesOfFace = &PumGetNumVerticesOfFace;
	interfaces.m_getPosition = &PumGetPosition;
	interfaces.m_getNormal = &PumGetNormal;
	interfaces.m_getTexCoord = &PumGetTexCoord;
	interfaces.m_setTSpaceBasic = &PumSetTangent;
	interfaces.m_setTSpace = nullptr;

	/* We need to precalculate the result buffer size, otherwise the reallocating of the output buffer will take forever. */
	size_t meshCnt = 0, outputSize = 0;
	for (const pum_mesh &mesh : data.Geometry)
	{
		if (mesh.HasNormals && mesh.HasTextureUvs && mesh.Topology == 3 && !mesh.HasTangents)
		{
			meshCnt++;
			const size_t oldStride = mesh.GetStride();
			const size_t newStride = oldStride + sizeof(Vector4);
			const size_t elementCnt = mesh.VertexViewSize / oldStride;
			outputSize += elementCnt * newStride;
		}
	}

	/* Indicate to the user that this action will take place and just convert for all meshes. */
	Log::Message("Generating tangents for %zu meshes using MikkTSpace.", meshCnt);
	BinaryWriter newData{ outputSize, Endian::Little };

	for (pum_mesh &mesh : data.Geometry)
	{
		/* Normals and texture coordinates are needed for tangent space calculation. */
		if (mesh.HasNormals && mesh.HasTextureUvs && mesh.Topology == 3)
		{
			/* Skip meshes that have tangents defined. */
			if (!mesh.HasTangents)
			{
				/* We use this struct to give all the static functions access to the models data. */
				MikkTSpaceUserData userData{ data, mesh };

				/* Actually calculate the tangents. */
				SMikkTSpaceContext context{};
				context.m_pUserData = &userData;
				context.m_pInterface = &interfaces;
				genTangSpaceDefault(&context);

				/* Set the tangents and copy over the indices if needed. */
				if (mesh.IndexMode != 2)
				{
					mesh.IndexViewStart = newData.GetSize();
					newData.Write(data.Data.GetData(), mesh.IndexViewStart, mesh.IndexViewSize);
				}

				/* Make sure that the mesh starts at an alligned offset. */
				const size_t zeroBytes = newData.GetSize() % (userData.stride + sizeof(Vector4));
				newData.Pad(zeroBytes);

				/* The end stride is just the stride with the position and normal size subtracted. */
				const size_t start = newData.GetSize();
				const size_t endStride = userData.stride - (sizeof(Vector3) << 1);
				const byte *raw = data.Data.GetData();

				for (size_t i = mesh.VertexViewStart, j = 0; i < mesh.VertexViewStart + mesh.VertexViewSize; j++)
				{
					/* Copy the position and the normal. */
					newData.Write(raw, i, sizeof(Vector3) << 1);
					i += sizeof(Vector3) << 1;

					/* Copy the generated tangent. */
					newData.Write(userData.Tangents[j]);

					/* Write the last part of the vertex to the output buffer. */
					newData.Write(raw, i, endStride);
					i += endStride;
				}

				/* Override the old parameters. */
				mesh.HasTangents = true;
				mesh.VertexViewStart = start;
				mesh.VertexViewSize = newData.GetSize() - start;
			}
		}
		else Log::Warning("Unable to generate tangents for model '%s' (mesh doesn't have normals or texture coordinates)!", mesh.Identifier.toUTF8().c_str());
	}

	/* Replace the old buffer with out new buffer. */
	data.Data = std::move(newData);
}