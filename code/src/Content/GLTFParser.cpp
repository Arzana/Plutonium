#include "Content/GLTFParser.h"
#include "Streams/FileReader.h"

#define COPY_PRIMITIVE_ATTRIBUTE(attr, ptr)																										\
					if (LoadMeshPrimitive(copy, bufferData, offset + internalOffset, stride, type, file, primitive, attr))						\
					{																															\
						ptr = new BufferAccessor(*result, type, internalOffset);																\
						internalOffset += type.GetSize();																						\
					}

namespace Pu
{
	/* Clears the vector if it has elements and logs a warning for clearning it. */
	template <typename element_t>
	inline void ClearVectorIfNeeded(vector<element_t> &vector)
	{
		if (vector.size() > 0)
		{
			Log::Warning("Non-empty vector passed to GLTF buffer parser, clearing vector before use!");
			vector.clear();
		}
	}

	/* Not all codepaths return a value, Log::Fatal will always throw. */
#pragma warning(push)
#pragma warning(disable:4715)
	inline PrimitiveTopology GLTFMode2Topology(GLTFMode mode)
	{
		switch (mode)
		{
		case Pu::GLTFMode::Points:
			return PrimitiveTopology::PointList;
		case Pu::GLTFMode::Line:
		case Pu::GLTFMode::LineLoop:	// Vulkan doesn't have a line loop so just default.
			return PrimitiveTopology::LineList;
		case Pu::GLTFMode::Triangles:
			return PrimitiveTopology::TriangleList;
		case Pu::GLTFMode::TriangleStrip:
			return PrimitiveTopology::TriangleStrip;
		case Pu::GLTFMode::TriangleFan:
			return PrimitiveTopology::TriangleFan;
		}

		Log::Fatal("Unknown GLTFMode passed!");
	}
#pragma warning(pop)

	/* Loads the parsed buffers into staging buffers for the user to use. */
	void InitializeStagingBuffers(const GLTFFile &file, byte **bufferData, const vector<std::reference_wrapper<Buffer>> &buffers, vector<StagingBuffer*> &stagingBuffers)
	{
		/* Parse all of the buffers. */
		for (size_t i = 0; i < file.Buffers.size(); i++)
		{
			/* Get the GLTF buffer header and the result staging buffer. */
			StagingBuffer *result = new StagingBuffer(buffers[i]);

			/* Load the data into the staging buffer. */
			result->Load(bufferData[i]);
			stagingBuffers.emplace_back(result);
		}
	}

	/* Moves a single mesh primitive attribute across buffers. */
	bool LoadMeshPrimitive(byte **source, byte **destination, size_t offset, size_t stride, FieldType &accessorType, const GLTFFile &file, const GLTFPrimitive &primitive, GLTFPrimitiveAttribute type)
	{
		/* Check if the primitive attribute is present. */
		size_t idx;
		if (primitive.TryGetAttribute(type, idx))
		{
			/* Gets the accessor and its view. */
			const GLTFAccessor &accessor = file.Accessors[idx];
			const GLTFBufferView &view = file.BufferViews[accessor.BufferView];

			/* Copy the data. */
			for (size_t i = 0, j = view.Start + accessor.Start, fieldStride = accessor.FieldType.GetSize(); i < accessor.Count; i++, j += fieldStride, offset += stride)
			{
				memcpy(destination[view.Buffer] + offset, source[view.Buffer] + j, fieldStride);
			}

			/* Set the indicators. */
			accessorType = accessor.FieldType;
			return true;
		}

		return false;
	}

	/* Move all the data around so it's faster to work with new format is described in GLTFMonsterBufferLayout. */
	void RelocateData(const GLTFFile & file, byte **bufferData, const vector<std::reference_wrapper<Buffer>> &buffers, vector<Mesh*> &meshes)
	{
		//TODO: for now just relocate the mesh data and zero out the rest.

		/* Make a copy of the source buffer to draw data from. */
		byte **copy = reinterpret_cast<byte**>(malloc(file.Buffers.size() * sizeof(byte*)));
		for (size_t i = 0; i < file.Buffers.size(); i++)
		{
			const size_t size = file.Buffers[i].Size;
			copy[i] = reinterpret_cast<byte*>(malloc(size));
			memcpy_s(copy[i], size, bufferData[i], size);
		}

		/* Copy all the mesh data to the correct place in the source buffer. */
		size_t offset = 0;
		for (const GLTFMesh &mesh : file.Meshes)
		{
			for (const GLTFPrimitive &primitive : mesh.Primitives)
			{
				/* All of the mesh data should be in the same file. */
				Buffer *firstBuffer = nullptr;
				size_t size = 0, stride = 0;

				/* Loop through all the attributes to figure out the final size and the stride. */
				for (const auto[type, idx] : primitive.Attributes)
				{
					const GLTFAccessor &accessor = file.Accessors[idx];
					const GLTFBufferView &view = file.BufferViews[accessor.BufferView];
					if (!firstBuffer)
					{
						firstBuffer = &buffers[view.Buffer].get();
					}

					/* Calculate the size this attribute will take in the buffer. */
					const size_t accessorStride = accessor.FieldType.GetSize();
					size += accessor.Count * accessorStride;
					stride += accessorStride;
				}

				/* Create the resulting mesh structure. */
				if (firstBuffer)
				{
					FieldType type;
					size_t internalOffset = 0;

					Mesh *result = new Mesh(*firstBuffer, offset, size, stride, GLTFMode2Topology(primitive.Mode));

					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::Position, result->pos);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::Normal, result->norm);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::Tangent, result->tang);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::TexCoord1, result->tex1);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::TexCoord2, result->tex2);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::Color, result->clr);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::Joints, result->jnts);
					COPY_PRIMITIVE_ATTRIBUTE(GLTFPrimitiveAttribute::Weights, result->wghs);

					offset += size;

					/* Create the index buffer view if needed. */
					if (primitive.HasIndices)
					{
						const GLTFAccessor &accessor = file.Accessors[primitive.Indices];
						const GLTFBufferView &view = file.BufferViews[accessor.BufferView];
						const size_t idxStride = accessor.FieldType.GetSize();
						
						vector<uint16> indices;
						memcpy(bufferData[view.Buffer] + offset, copy[view.Buffer] + view.Start + accessor.Start, view.Length);

						result->idxView = new BufferView(*firstBuffer, offset, view.Length, idxStride);
						result->idxAcce = new BufferAccessor(*result->idxView, accessor.FieldType, 0);

						offset += view.Length;
					}

					meshes.emplace_back(result);
				}
			}
		}

		/* Free the temporary buffers. */
		for (size_t i = 0; i < file.Buffers.size(); i++) free(copy[i]);
		free(copy);
	}

	void _CrtLoadAndParseGLTF(const GLTFFile & file, const vector<std::reference_wrapper<Buffer>> & buffers, vector<StagingBuffer*> & stagingBuffers, vector<Mesh*> & meshes)
	{
		/* Make sure the output buffer is valid. */
		ClearVectorIfNeeded(meshes);

		byte **data = reinterpret_cast<byte**>(malloc(file.Buffers.size() * sizeof(byte*)));

		/*
		Load all binary files and parse them.
		Most GLTF files will only have one buffer so it's a waste to multithread this.
		*/
		for (size_t i = 0; i < file.Buffers.size(); i++)
		{
			const GLTFBuffer &info = file.Buffers[i];

			/*
			Attempt to open the file and read all its contents.
			The size of the buffer is stored in the GLTF header so we can read it more effeciently.
			So just preallocate the buffer and use Read instead of ReadToEnd to avoid file seeking.
			*/
			FileReader reader(info.Uri);
			data[i] = reinterpret_cast<byte*>(malloc(info.Size));
			reader.Read(data[i], 0, info.Size);
		}

		/* Rearranges the data within the buffers and create the meshes and animations. */
		RelocateData(file, data, buffers, meshes);
		/* Create the required staging buffers and load the data into them. */
		InitializeStagingBuffers(file, data, buffers, stagingBuffers);

		/* Free the temporary buffers. */
		for (size_t i = 0; i < file.Buffers.size(); i++) free(data[i]);
		free(data);
	}
}