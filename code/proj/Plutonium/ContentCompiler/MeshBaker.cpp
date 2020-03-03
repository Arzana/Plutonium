#include "MeshBaker.h"
#include <Core/Diagnostics/Stopwatch.h>

using namespace Pu;

using Range = std::pair<size_t, size_t>;

struct MeshInfo
{
	pum_mesh Mesh;
	vector<Range> Indices;
	vector<Range> Vertices;
	vector<uint32> Ids;
	vector<size_t> Offsets;

	MeshInfo(const pum_mesh &mesh, uint32 oldIdx)
		: Mesh(mesh)
	{
		Ids.emplace_back(oldIdx);
	}

	size_t GetIndexCount(void) const
	{
		const size_t stride = Mesh.GetIdxStride();
		size_t result = Mesh.IndexViewSize / stride;
		for (auto[start, size] : Indices) result += size / stride;
		return result;
	}
};

/* Defines whether two meshes have the same primitives. */
bool AreMeshesCompatible(const pum_mesh &a, const pum_mesh &b)
{
	return a.HasNormals == b.HasNormals
		&& a.HasTangents == b.HasTangents
		&& a.HasTextureUvs == b.HasTextureUvs
		&& a.HasVertexColors == b.HasVertexColors
		&& a.HasJoints == b.HasJoints
		&& a.IndexMode == b.IndexMode;
}

/* Defines whether two meshes can be baked together. */
bool CanBake(const pum_mesh &a, const pum_mesh &b)
{
	if (a.WriteMaterialIndex == b.WriteMaterialIndex)
	{
		if (!a.WriteMaterialIndex) return AreMeshesCompatible(a, b);
		else if (a.Material == b.Material) return AreMeshesCompatible(a, b);
	}

	return false;
}

void UpdateNodeReferences(PumIntermediate &data, const vector<MeshInfo> &newMeshes)
{
	uint32 i = 0;
	for (const MeshInfo &info : newMeshes)
	{
		for (pum_node &node : data.Nodes)
		{
			/* We can skip nodes that have no mesh reference. */
			if (!node.WriteMeshIndex) continue;

			for (size_t j : info.Ids)
			{
				/* Simply set the new index if the index was baked into a new mesh. */
				if (node.Mesh == j)
				{
					node.Mesh = i;
					break;
				}
			}
		}

		++i;
	}
}

template <typename in_t, typename out_t>
void WriteIndexGroup(const byte *raw, size_t start, size_t size, size_t offset, BinaryWriter &writer)
{
	const in_t *ptr = reinterpret_cast<const in_t*>(raw + start);
	const in_t *end = reinterpret_cast<const in_t*>(raw + start + size);

	for (; ptr < end; ptr++)
	{
		writer.Write(static_cast<out_t>(*ptr + offset));
	}
}

template <typename in_t, typename out_t>
size_t WriteIndices(const byte *raw, MeshInfo &mesh, BinaryWriter &writer)
{
	/* Write the old indices to the output. */
	writer.Align(sizeof(out_t));
	const size_t newStart = writer.GetSize();
	WriteIndexGroup<in_t, out_t>(raw, mesh.Mesh.IndexViewStart, mesh.Mesh.IndexViewSize, 0, writer);

	/* Write the new indices to the output. */
	size_t i = 0;
	for (auto[start, size] : mesh.Indices) WriteIndexGroup<in_t, out_t>(raw, start, size, mesh.Offsets[i++], writer);
	return newStart;
}

void BakeMeshes(PumIntermediate & data, const string & name)
{
	Stopwatch sw = Stopwatch::StartNew();
	vector<MeshInfo> newMeshes;
	size_t bakeCnt = 0;

	/* Handle all of the meshes. */
	while (!data.Geometry.empty())
	{
		const pum_mesh &mesh = data.Geometry.back();
		const uint32 idx = static_cast<uint32>(data.Geometry.size() - 1);

		/* Check if the current mesh can be baked with any of the old meshes. */
		bool exclusive = true;
		for (MeshInfo &test : newMeshes)
		{
			if (CanBake(mesh, test.Mesh))
			{
				/* We can only bake another mesh into this if the index count allows it. */
				if (test.Mesh.IndexMode != 2 && (test.GetIndexCount() > maxv<uint32>())) continue;

				/* Add an offset that should be added to all indices in the new mesh. */
				size_t offset = test.Mesh.GetVrtxCount();
				for (auto[start, size] : test.Vertices) offset += size / test.Mesh.GetVrtxStride();
				test.Offsets.emplace_back(offset);

				/* Just add the mesh range and it's ID to the mesh information. */
				exclusive = false;
				test.Indices.emplace_back(std::make_pair(mesh.IndexViewStart, mesh.IndexViewSize));
				test.Vertices.emplace_back(std::make_pair(mesh.VertexViewStart, mesh.VertexViewSize));
				test.Ids.emplace_back(idx);

				/* Merge the bounds of the mesh, so it can be properly culled. */
				test.Mesh.Bounds = test.Mesh.Bounds.Merge(mesh.Bounds);
				++bakeCnt;
				break;
			}
		}

		/* Add the old mesh to the list if it could not be bakes and remove it from the old list. */
		if (exclusive) newMeshes.emplace_back(mesh, idx);
		data.Geometry.pop_back();
	}

	if (bakeCnt) Log::Message("Statically baking %zu meshes.", bakeCnt);
	else
	{
		/* Early out if there are no meshes to bake. */
		for (const MeshInfo &info : newMeshes) data.Geometry.emplace_back(info.Mesh);
		return;
	}

	/* Update all the node references to the meshes. */
	UpdateNodeReferences(data, newMeshes);

	/* Copy the new meshes to the output buffer. */
	BinaryWriter writer{ data.Data.GetSize(), Endian::Little };
	const byte *raw = data.Data.GetData();
	for (MeshInfo &info : newMeshes)
	{
		size_t newStart;

		/* Write indices if needed. */
		if (info.Mesh.IndexMode != 2)
		{
			/* Write the indices in either 16-bits or 32-bits. */
			if (info.GetIndexCount() < maxv<uint16>())
			{
				if (info.Mesh.IndexMode == 0) newStart = WriteIndices<uint16, uint16>(raw, info, writer);
				else newStart = WriteIndices<uint32, uint16>(raw, info, writer);
				info.Mesh.IndexMode = 0;
			}
			else
			{
				if (info.Mesh.IndexMode == 0) newStart = WriteIndices<uint16, uint32>(raw, info, writer);
				else newStart = WriteIndices<uint32, uint32>(raw, info, writer);
				info.Mesh.IndexMode = 1;
			}

			/* Update the index view info. */
			info.Mesh.IndexViewStart = newStart;
			info.Mesh.IndexViewSize = writer.GetSize() - newStart;
		}

		/* Write the origional vertex information. */
		writer.Align(info.Mesh.GetVrtxStride());
		newStart = writer.GetSize();
		writer.Write(raw, info.Mesh.VertexViewStart, info.Mesh.VertexViewSize);

		/* Write the baked vertices. */
		for (auto[start, size] : info.Vertices) writer.Write(raw, start, size);

		/* Update the vertex view info and add the mesh back to the list. */
		info.Mesh.VertexViewStart = newStart;
		info.Mesh.VertexViewSize = writer.GetSize() - newStart;
		data.Geometry.emplace_back(info.Mesh);
	}

	/* Override the old data. */
	data.Data = std::move(writer);
	Log::Message("Finished baking meshes for model '%s', took %f seconds.", name.c_str(), sw.SecondsAccurate());
}