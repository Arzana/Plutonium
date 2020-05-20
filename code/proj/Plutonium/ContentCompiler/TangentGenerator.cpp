#include "TangentGenerator.h"
#include "IndicesGenerator.h"
#include <Streams/BinaryReader.h>
#include <mikktspace/mikktspace.h>
#include <Core/Diagnostics/Stopwatch.h>
#include <Core/Threading/Tasks/Scheduler.h>

using namespace Pu;

class MikkTSpaceUserData
{
public:
	pum_mesh &Mesh;

	MikkTSpaceUserData(const PumIntermediate &data, pum_mesh &mesh)
		: Mesh(mesh), oldStride(mesh.GetVrtxStride()), idxReader(nullptr),
		vrtxReader(data.Data.GetData() + mesh.VertexViewStart, mesh.VertexViewSize)
	{
		/* We need to create an entire new vertex array, because the indices will change. */
		newStride = oldStride + (sizeof(Vector4) * !mesh.HasTangents);
		newVrtxData = reinterpret_cast<byte*>(malloc(GetVrtxCount() * newStride));

		/* Only set an index reader if the mesh has one. */
		if (mesh.IndexMode != 2)
		{
			idxReader = new BinaryReader(data.Data.GetData() + mesh.IndexViewStart, mesh.IndexViewSize);
			idxStride = mesh.GetIdxStride();
		}
	}

	~MikkTSpaceUserData(void)
	{
		free(newVrtxData);
		if (idxReader) delete idxReader;
	}

	static size_t GetFaceCount(const SMikkTSpaceContext *context)
	{
		return Get(context).GetVrtxCount() / 3;
	}

	static Vector3 GetPosition(const SMikkTSpaceContext *context, int faceIdx, int vertIdx)
	{
		/* The position is first in the vertex so we don't have to add any offset. */
		MikkTSpaceUserData &instance = Get(context);
		instance.Seek(faceIdx, vertIdx, 0);
		return instance.vrtxReader.PeekVector3();
	}

	static Vector3 GetNormal(const SMikkTSpaceContext *context, int faceIdx, int vertIdx)
	{
		/* The normal is second so we only add the offset from the postion. */
		MikkTSpaceUserData &instance = Get(context);
		instance.Seek(faceIdx, vertIdx, sizeof(Vector3));
		return instance.vrtxReader.PeekVector3();
	}

	static Vector2 GetTexCoord(const SMikkTSpaceContext *context, int faceIdx, int vertIdx)
	{
		/* The texture coordinate if 3rd (tangent not included for obvious reasons). */
		MikkTSpaceUserData &instance = Get(context);
		instance.Seek(faceIdx, vertIdx, sizeof(Vector3) << 1);
		return instance.vrtxReader.PeekVector2();
	}

	static inline void WriteTangent(const SMikkTSpaceContext *context, int faceIdx, int vertIdx, Vector4 tangent)
	{
		/* We write the entire vertex to the new buffer now. */
		Get(context).WriteInternal(faceIdx, vertIdx, tangent);
	}

	void Write(BinaryWriter &writer, std::mutex &lock)
	{
		/* The generate indices function writes the indices and vertices to the output buffer. */
		Mesh.VertexViewSize = GetVrtxCount() * newStride;
		Mesh.HasTangents = true;
		GenerateIndices(Mesh, newVrtxData, writer, &lock);
	}

private:
	byte *newVrtxData;
	BinaryReader vrtxReader;
	BinaryReader *idxReader;
	size_t oldStride, newStride, idxStride, endStride;

	static inline MikkTSpaceUserData& Get(const SMikkTSpaceContext *context)
	{
		return *reinterpret_cast<MikkTSpaceUserData*>(context->m_pUserData);
	}

	size_t GetVrtxCount(void) const
	{
		/* Get the vertex count through either the indices or the vertices. */
		return Mesh.IndexMode != 2 ? Mesh.GetIndexCount() : Mesh.GetVrtxCount();
	}

	size_t GetOffset(int faceIdx, int vertIdx)
	{
		if (idxReader)
		{
			/* We need to get the vertex offset from the index buffer. */
			idxReader->Seek(static_cast<size_t>((faceIdx * idxStride * 3) + (vertIdx * idxStride)));
			const uint32 i = Mesh.IndexMode == 0 ? idxReader->ReadUInt16() : idxReader->ReadUInt32();

			return i * oldStride;
		}
		else
		{
			/* This operation is easy if no index buffer is present. */
			return (faceIdx * oldStride * 3) + (vertIdx * oldStride);
		}
	}

	void Seek(int faceIdx, int vertIdx, size_t offset)
	{
		/* Use the faster seek option that the binary reader offers us. */
		vrtxReader.Seek(GetOffset(faceIdx, vertIdx) + offset);
	}

	void WriteInternal(int faceIdx, int vertIdx, Vector4 tangent)
	{
		/* We always start with a position and normal, but the data after the tangent is mesh dependent. */
		constexpr size_t startStride = sizeof(Vector3) << 1;
		const size_t endStride = oldStride - startStride - (sizeof(Vector4) * Mesh.HasTangents);

		/* The index into out destination buffer is easy, but the index to our source buffer might come from a indexed buffer. */
		size_t i = (faceIdx * newStride * 3) + (vertIdx * newStride);
		size_t j = GetOffset(faceIdx, vertIdx);
		const byte *oldSrc = reinterpret_cast<const byte*>(vrtxReader.GetData());

		/* Copy the position and normal over. */
		memcpy(newVrtxData + i, oldSrc + j, startStride);
		i += startStride;
		j += startStride;

		/* Copy the tangent. */
		memcpy(newVrtxData + i, tangent.f, sizeof(Vector4));
		i += sizeof(Vector4);
		j += sizeof(Vector4) * Mesh.HasTangents;

		/* Copy the remainder of the vertex. */
		memcpy(newVrtxData + i, oldSrc + j, endStride);
	}
};

/* Defines a single instance of a tangent generation task for a specific mesh. */
class GenerateTask
	: public Task
{
public:
	GenerateTask(const PumIntermediate &data, pum_mesh &mesh, SMikkTSpaceInterface &delegates, std::mutex &lock, std::atomic_uint32_t &counter, BinaryWriter &writer)
		: data(data), mesh(mesh), delegates(delegates), writeLock(lock), writer(writer), running(counter)
	{
		running++;
	}

	~GenerateTask(void)
	{
		--running;
	}

	virtual Result Execute(void) override
	{
		/* We allocate the user data in the thread local part, to save on startup performance. */
		MikkTSpaceUserData userData{ data, mesh };
		SMikkTSpaceContext context{ &delegates, &userData };

		if (genTangSpaceDefault(&context))
		{
			/* We need to lock the writer on the actual writes, so pass a shared lock. */
			userData.Write(writer, writeLock);
		}
		else Log::Error("MikkTSpace failed for mesh '%s'!", mesh.Identifier.toUTF8().c_str());

		/* We're always done at this point. */
		return Result::AutoDelete();
	}

private:
	pum_mesh &mesh;
	const PumIntermediate &data;
	SMikkTSpaceInterface &delegates;
	std::atomic_uint32_t& running;
	std::mutex &writeLock;
	BinaryWriter &writer;
};

int PumGetNumFaces(const SMikkTSpaceContext *context)
{
	return static_cast<int>(MikkTSpaceUserData::GetFaceCount(context));
}

int PumGetNumVerticesOfFace(const SMikkTSpaceContext *context, int)
{
	/* PuM only allows for triangles. */
	return 3;
}

void PumGetPosition(const SMikkTSpaceContext *context, float posOut[3], int faceIdx, int vertIdx)
{
	const Vector3 pos = MikkTSpaceUserData::GetPosition(context, faceIdx, vertIdx);
	memcpy(posOut, pos.f, sizeof(Vector3));
}

void PumGetNormal(const SMikkTSpaceContext *context, float normalOut[3], int faceIdx, int vertIdx)
{
	const Vector3 normal = MikkTSpaceUserData::GetNormal(context, faceIdx, vertIdx);
	memcpy(normalOut, normal.f, sizeof(Vector3));
}

void PumGetTexCoord(const SMikkTSpaceContext *context, float coordOut[2], int faceIdx, int vertIdx)
{
	const Vector2 coord = MikkTSpaceUserData::GetTexCoord(context, faceIdx, vertIdx);
	memcpy(coordOut, coord.f, sizeof(Vector2));
}

void PumSetTangent(const SMikkTSpaceContext *context, const float tangent[3], float sign, int faceIdx, int vertIdx)
{
	/* We need to invert the sign because MikkTSpace has a left-handed coordinate system and we require a righ-handed one. */
	const Vector4 value{ tangent[0], tangent[1], tangent[2], -sign };
	MikkTSpaceUserData::WriteTangent(context, faceIdx, vertIdx, value);
}

void LogMeshConversion(PumIntermediate &data, bool newTangents, bool recalculate)
{
	size_t cnt = 0;

	for (const pum_mesh &mesh : data.Geometry)
	{
		/* Check whether the mesh can be converted. */
		if (mesh.HasNormals && mesh.HasTextureUvs)
		{
			cnt += (newTangents && !mesh.HasTangents) || (recalculate && mesh.HasTangents);
		}
	}

	Log::Message("Generating new tangents for %zu meshes using MikkTSpace.", cnt);
}

int GenerateTangents(PumIntermediate & data, const CLArgs & args)
{
	Stopwatch sw = Stopwatch::StartNew();

	/* Set all of the delegates. */
	SMikkTSpaceInterface interfaces{};
	interfaces.m_getNumFaces = &PumGetNumFaces;
	interfaces.m_getNumVerticesOfFace = &PumGetNumVerticesOfFace;
	interfaces.m_getPosition = &PumGetPosition;
	interfaces.m_getNormal = &PumGetNormal;
	interfaces.m_getTexCoord = &PumGetTexCoord;
	interfaces.m_setTSpaceBasic = &PumSetTangent;
	interfaces.m_setTSpace = nullptr;

	/* Log how many conversions will take place. */
	LogMeshConversion(data, args.CreateTangents, args.RecalcTangents);
	BinaryWriter writer{ 0u, Endian::Little };
	std::mutex writeLock;
	vector<GenerateTask*> tasks;

	int result = EXIT_SUCCESS;
	size_t reserveSize = 0;
	std::atomic_uint32_t tasksRunning{ 0 };

	for (pum_mesh &mesh : data.Geometry)
	{
		const string name = mesh.Identifier.toUTF8();

		/* We can skip any mesh that already has tangents. */
		if (mesh.HasTangents && !args.RecalcTangents)
		{
			writeLock.lock();
			if (mesh.IndexMode != 2)
			{
				/* Copy the index buffer to the output. */
				writer.Align(mesh.GetIdxStride());
				mesh.IndexViewStart = writer.GetSize();
				writer.Write(data.Data.GetData(), mesh.IndexViewStart, mesh.IndexViewSize);
			}

			/* Copy the vertex buffer to the output. */
			writer.Align(mesh.GetVrtxStride());
			mesh.VertexViewStart = writer.GetSize();
			writer.Write(data.Data.GetData(), mesh.VertexViewStart, mesh.VertexViewSize);
			writeLock.unlock();
		}
		else if (mesh.HasNormals && mesh.HasTextureUvs && mesh.Topology == 3)
		{
			/* Just spwan a tasks that will generate the tangents (and maybe indices) for us. */
			GenerateTask *task = new GenerateTask(data, mesh, interfaces, writeLock, tasksRunning, writer);
			tasks.emplace_back(task);
			args.Scheduluer->Spawn(*task);

			/* Add the maximum required amount of bytes to the output buffer. */
			const size_t elementSize = mesh.IndexMode != 2 ? mesh.GetIndexCount() : mesh.GetVrtxCount();
			reserveSize += elementSize * mesh.GetVrtxStride() + (sizeof(Vector4) * !mesh.HasTangents);
		}
		else Log::Warning("Unable to generate tangents for mesh '%s' (mesh doesn't have normals, texture coordinates, or isn't a triangle list)!", name.c_str());
	}

	/* Reserve the writer to make sure it doesn't reallocate during it's runtime. */
	writeLock.lock();
	writer.EnsureCapacity(reserveSize);
	writeLock.unlock();

	/* Wait for all of the tasks to complete. */
	while (tasksRunning.load()) PuThread::Sleep(100);

	/* Override the old data. */
	writeLock.lock();
	data.Data = std::move(writer);
	writeLock.unlock();

	Log::Message("Finished generating tangents for model '%s', took %f seconds.", args.DisplayName.c_str(), sw.SecondsAccurate());
	return result;
}