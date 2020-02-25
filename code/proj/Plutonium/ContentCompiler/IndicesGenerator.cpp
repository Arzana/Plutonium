#include "IndicesGenerator.h"
#include <set>

/* Just an ease of use define. */
#define CAST(input, offset, type)		*reinterpret_cast<const type*>(reinterpret_cast<const byte*>(input) + offset)
constexpr float EPS = 0.001f;

using namespace Pu;

/* Defines the biggest vertex that PuM supports. */
struct BigVertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector4 Tangent;
	Vector2 TexCoord;
	uint32 Color;
	uint64 Joints;
	Vector4 Weights;

	BigVertex(void)
		: Color(0), Joints(0)
	{}

	bool operator <(const BigVertex &other) const
	{
		if (nrlyneql(Position, other.Position, EPS)) return Position < other.Position;
		if (nrlyneql(Normal, other.Normal, EPS)) return Normal < other.Normal;
		if (nrlyneql(Tangent, other.Tangent, EPS)) return Tangent < other.Tangent;
		if (nrlyneql(TexCoord, other.TexCoord, EPS)) return TexCoord < other.TexCoord;
		if (Color != other.Color) return Color < other.Color;
		if (Joints != other.Joints) return Joints < other.Joints;
		if (nrlyneql(Weights, other.Weights, EPS)) return Weights < other.Weights;
		return false;
	}
};

/* Defines our temporary storage medium. */
using ExclusiveVertex = std::pair<BigVertex, uint32>;

struct VertexComparitor
{
	inline bool operator()(const ExclusiveVertex &v, const ExclusiveVertex &w) const
	{
		return v.first < w.first;
	}
};

template <typename T>
void WriteIndices(pum_mesh &mesh, BinaryWriter &writer, const vector<uint32> &indices, char mode)
{
	writer.Align(sizeof(T));
	mesh.IndexMode = mode;
	mesh.IndexViewStart = writer.GetSize();
	mesh.IndexViewSize = indices.size() * sizeof(T);

	for (uint32 i : indices) writer.Write(static_cast<T>(i));
}

void GenerateIndices(pum_mesh & mesh, const void * vertices, BinaryWriter & writer, std::mutex *lock)
{
	/* Get the amount of input vertices and allocate a temporary set used for caomparisons. */
	const size_t stride = mesh.GetVrtxStride();
	const size_t inputCount = mesh.VertexViewSize / stride;

	std::set<ExclusiveVertex, VertexComparitor> set;
	vector<ExclusiveVertex> tmp;
	vector<uint32> indices;

	/* Allocate the maximum possible size beforehand to save on allocations. */
	tmp.reserve(inputCount);
	indices.reserve(inputCount);

	uint32 k = 0;
	for (size_t i = 0, j = 0; i < inputCount; i++)
	{
		BigVertex vrtx;

		/* The position is always set a vertex. */
		vrtx.Position = CAST(vertices, j, Vector3);
		j += sizeof(Vector3);

		/* Set the normal if needed. */
		if (mesh.HasNormals)
		{
			vrtx.Normal = CAST(vertices, j, Vector3);
			j += sizeof(Vector3);
		}

		/* Set the tangent if needed. */
		if (mesh.HasTangents)
		{
			vrtx.Tangent = CAST(vertices, j, Vector4);
			j += sizeof(Vector4);
		}

		/* Set the texture coordinates if needed. */
		if (mesh.HasTextureUvs)
		{
			vrtx.TexCoord = CAST(vertices, j, Vector2);
			j += sizeof(Vector2);
		}

		/* Set the vertex color if needed. */
		if (mesh.HasVertexColors)
		{
			vrtx.Color = CAST(vertices, j, uint32);
			j += sizeof(uint32);
		}

		/* Set the joints if needed. */
		if (mesh.HasJoints)
		{
			if (mesh.HasJoints == 1)
			{
				/* We're just comparing, so store them packed, so we don't have to unpack them. */
				vrtx.Joints = CAST(vertices, j, uint32);
				j += sizeof(uint32);
			}
			else if (mesh.HasJoints == 2)
			{
				vrtx.Joints = CAST(vertices, j, uint64);
				j += sizeof(uint64);
			}

			vrtx.Weights = CAST(vertices, j, Vector4);
			j += sizeof(Vector4);
		}

		/* 
		Either emplace the index to the index list if the vertex already exists.
		We use a set to speed up the search, the vector is O(n^2) whilst the set is O(n log n).
		*/
		decltype(set)::iterator it = set.find(std::make_pair(vrtx, 0u));
		if (it != set.end()) indices.emplace_back(it->second);
		else
		{
			/*
			We use the set to check if a vertex already exists, 
			this set is however sorted, so our indices will be wrong.
			Therefor we store it in a vector as well, but this is only used for output.
			*/
			set.emplace(std::make_pair(vrtx, k));
			tmp.emplace_back(std::make_pair(vrtx, k));
			indices.emplace_back(k++);
		}
	}

	/* We don't need to compare anymore, so deallocate the set to save on memory. */
	set.clear();

	/* All our work might have made the mesh bigger than it had to be, if this is the case, just void our effords. */
	size_t reserveSize = indices.size() * (indices.size() < maxv<uint16>() ? sizeof(uint16) : sizeof(uint32));
	reserveSize += tmp.size() * stride;
	if (reserveSize >= mesh.VertexViewSize)
	{
		/* Clear the buffers to get some more memory (in case we need it). */
		indices.clear();
		tmp.clear();

		/* Lock the writer if it was specified. */
		if (lock) lock->lock();
		writer.Align(stride);
		mesh.IndexMode = 2;
		mesh.VertexViewStart = writer.GetSize();
		writer.Write(reinterpret_cast<const byte*>(vertices), 0, stride * inputCount);
		if (lock) lock->unlock();
		return;
	}

	/* Allocate all the memory for the writer in one go, to save on allocation time later. */
	if (lock) lock->lock();
	writer.EnsureCapacity(reserveSize);

	/* We can use a small index list if the indices never passed the 16-bit limit. */
	if (indices.size() < maxv<uint16>()) WriteIndices<uint16>(mesh, writer, indices, 0);
	else WriteIndices<uint32>(mesh, writer, indices, 1);
	indices.clear();

	/* Write only the vertices to the result buffer. */
	writer.Align(stride);
	mesh.VertexViewStart = writer.GetSize();
	mesh.VertexViewSize = tmp.size() * stride;
	for (const auto &[vrtx, idx] : tmp)
	{
		writer.Write(vrtx.Position);
		if (mesh.HasNormals) writer.Write(vrtx.Normal);
		if (mesh.HasTangents) writer.Write(vrtx.Tangent);
		if (mesh.HasTextureUvs) writer.Write(vrtx.TexCoord);
		if (mesh.HasVertexColors) writer.Write(vrtx.Color);

		if (mesh.HasJoints)
		{
			/* We don't have to worry about the endianness as they are loaded with the same as they're stored. */
			if (mesh.HasJoints == 1) writer.Write(static_cast<uint32>(vrtx.Joints));
			else if (mesh.HasJoints == 2) writer.Write(vrtx.Joints);

			writer.Write(vrtx.Weights);
		}
	}

	/* Only log if we actually use the indices. */
	if (lock)lock->unlock();
	const string name = mesh.Identifier.toUTF8();
	Log::Message("Generated indices for mesh '%s'.", name.c_str());
}