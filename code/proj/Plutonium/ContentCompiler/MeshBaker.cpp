#include "MeshBaker.h"
#include <Core/Diagnostics/Stopwatch.h>
#include <Core/Diagnostics/Profiler.h>

using namespace Pu;

/* 
Converts the specified indices to normalized indices (always starts at 0).
This makes them relative to the mesh indead of all the vertices.
We can simply do this by subtracting the minimum index from all the indices.
*/
template <typename index_type>
void NormalizeIndices(BinaryWriter &writer, const void *raw, size_t size)
{
	const index_type *indices = reinterpret_cast<const index_type*>(raw);
	const size_t count = size / sizeof(index_type);
	
	index_type min = maxv<index_type>();
	for (size_t i = 0; i < count; i++)
	{
		const index_type cur = indices[i];
		if (cur < min) min = cur;
	}

	for (size_t i = 0; i < count; i++)
	{
		writer.Write(static_cast<index_type>(indices[i] - min));
	}
}

/* We need to move the indices from their default offset to a normalized offset. */
void ProcessIndices(BinaryWriter &writer, const void *raw, size_t size, char type)
{
	if (type == 0) NormalizeIndices<uint16>(writer, raw, size);
	else NormalizeIndices<uint32>(writer, raw, size);
}

/* Baking meshes just means that we sort all of the meshes on their material and orgazite their buffers views to allow for minimal bind calls. */
void BakeMeshes(PumIntermediate & data, const Pu::string & name)
{
	Stopwatch sw = Stopwatch::StartNew();
	Profiler::Begin("Mesh optimization");

	std::map<size_t, vector<uint32>> vertexViews;
	std::map<size_t, vector<uint32>> indexViews;
	BinaryWriter writer{ data.Data.GetSize(), Endian::Little };

	/* Initialize the vertex and index views. */
	uint32 i = 0;
	for (const pum_mesh &mesh : data.Geometry)
	{
		const size_t vrtxStride = mesh.GetVrtxStride();
		const size_t idxStride = mesh.GetIdxStride();

		/* Update the vertex view list. */
		decltype(vertexViews)::iterator it = vertexViews.find(vrtxStride);
		if (it != vertexViews.end()) it->second.emplace_back(i);
		else vertexViews.emplace(vrtxStride, vector<uint32>{ i });

		/* Update the index view list. */
		if (mesh.IndexMode != 2)
		{
			decltype(indexViews)::iterator it = indexViews.find(idxStride);
			if (it != indexViews.end()) it->second.emplace_back(i);
			else indexViews.emplace(idxStride, vector<uint32>{ i });
		}

		++i;
	}

	/* Create a new view for every found vertex format. */
	i = 0;
	for (const auto &[stride, meshes] : vertexViews)
	{
		/* Make sure we start at an aligned offset. */
		writer.Align(stride);
		pum_view view{ writer.GetSize() };

		/* Copy all the mesh vertices. */
		for (uint32 j : meshes)
		{
			pum_mesh &mesh = data.Geometry[j];
			const size_t newStart = writer.GetSize() - view.Offset;
			writer.Write(data.Data.GetData(), mesh.VertexViewStart, mesh.VertexViewSize);

			/* Update the view index and the new offset into the buffer. */
			mesh.VertexViewStart = newStart;
			mesh.VertexView = i;
		}

		/* Finalize the view and add it to the result. */
		view.Size = writer.GetSize() - view.Offset;
		data.Views.emplace_back(view);
		++i;
	}

	/* Create a new view for every found index format. */
	for (const auto &[stride, meshes] : indexViews)
	{
		/* Make sure we start at an aligned offset. */
		writer.Align(stride);
		pum_view view{ writer.GetSize() };
		size_t base = 0;

		/* Copy all the mesh indices. */
		for (uint32 j : meshes)
		{
			pum_mesh &mesh = data.Geometry[j];
			if (mesh.IndexMode == 2) continue;

			const size_t newStart = writer.GetSize() - view.Offset;
			ProcessIndices(writer, data.Data.GetData() + mesh.IndexViewStart, mesh.IndexViewSize, mesh.IndexMode);

			/* Update the view index and the new offset into the buffer. */
			mesh.IndexViewStart = newStart;
			mesh.IndexView = i;
		}

		/* Finalize the view and add it to the result. */
		view.Size = writer.GetSize() - view.Offset;
		data.Views.emplace_back(view);
		++i;
	}

	/* Replace to old data buffer with the new alligned data. */
	data.Data = std::move(writer);

	/* 
	We sort on 3 priorities:
	Vertex view is least likely to change so this is our first priority.
	Index view is also pretty unlikely to change, but a bit more than vertex format, so second priority.
	Material is quite likely to change, so this is our last priority.
	*/
	data.Geometry.sort([](const pum_mesh &a, const pum_mesh &b)
	{
		if (a.VertexView != b.VertexView) return a.VertexView < b.VertexView;
		if (a.IndexMode != b.IndexMode) return a.IndexMode < b.IndexMode;
		return a.Material < b.Material;
	});

	Log::Message("Done optimizing %zu meshes for %s, took %.3f seconds.", data.Geometry.size(), name.c_str(), sw.SecondsAccurate());
	Profiler::End();
}