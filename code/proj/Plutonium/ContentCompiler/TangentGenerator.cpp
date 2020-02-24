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
		newStride = oldStride + sizeof(Vector4);
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
		if (Mesh.IndexMode != 2) return Mesh.GetIndexCount();
		else return (Mesh.VertexViewSize / oldStride);
	}

	size_t GetOffset(int faceIdx, int vertIdx)
	{
		if (idxReader)
		{
			/* We need to get the vertex offset from the index buffer. */
			idxReader->Seek(SeekOrigin::Begin, static_cast<int64>((faceIdx * idxStride * 3) + (vertIdx * idxStride)));
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
		const size_t i = GetOffset(faceIdx, vertIdx) + offset;
		vrtxReader.Seek(SeekOrigin::Begin, static_cast<int64>(i));
	}

	void WriteInternal(int faceIdx, int vertIdx, Vector4 tangent)
	{
		/* We always start with a position and normal, but the data after the tangent is mesh dependent. */
		constexpr size_t startStride = sizeof(Vector3) << 1;
		const size_t endStride = oldStride - startStride;

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

		/* Copy the remainder of the vertex. */
		memcpy(newVrtxData + i, oldSrc + j, endStride);
	}
};

/* Defines a single instance of a tangent generation task for a specific mesh. */
class GenerateTask
	: public Task
{
public:
	GenerateTask(const PumIntermediate &data, pum_mesh &mesh, SMikkTSpaceInterface &delegates, std::mutex &lock, BinaryWriter &writer)
		: data(data), mesh(mesh), delegates(delegates), running(true), writeLock(lock), writer(writer)
	{}

	inline bool IsWorking(void) const
	{
		return running.load();
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
		running.store(false);
		return Result::Default();
	}

private:
	pum_mesh &mesh;
	const PumIntermediate &data;
	SMikkTSpaceInterface &delegates;
	std::atomic_bool running;
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

void PumGetNormal(const SMikkTSpaceContext *context, float normalOut[3], const int faceIdx, const int vertIdx)
{
	const Vector3 normal = MikkTSpaceUserData::GetNormal(context, faceIdx, vertIdx);
	memcpy(normalOut, normal.f, sizeof(Vector3));
}

void PumGetTexCoord(const SMikkTSpaceContext *context, float coordOut[2], const int faceIdx, const int vertIdx)
{
	const Vector2 coord = MikkTSpaceUserData::GetTexCoord(context, faceIdx, vertIdx);
	memcpy(coordOut, coord.f, sizeof(Vector2));
}

void PumSetTangent(const SMikkTSpaceContext *context, const float tangent[3], const float sign, const int faceIdx, const int vertIdx)
{
	const Vector4 value{ tangent[0], tangent[1], tangent[2], sign };
	MikkTSpaceUserData::WriteTangent(context, faceIdx, vertIdx, value);
}

void LogMeshConversion(PumIntermediate &data)
{
	size_t cnt = 0;

	for (const pum_mesh &mesh : data.Geometry)
	{
		cnt += mesh.HasNormals && mesh.HasTextureUvs && !mesh.HasTangents && mesh.Topology == 3;
	}

	Log::Message("Generating tangents for %zu meshes using MikkTSpace.", cnt);
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
	LogMeshConversion(data);
	BinaryWriter writer{ 0u, Endian::Little };
	std::mutex writeLock;
	vector<GenerateTask*> tasks;

	int result = EXIT_SUCCESS;
	for (pum_mesh &mesh : data.Geometry)
	{
		const string name = mesh.Identifier.toUTF8();

		/* We can skip any mesh that already has tangents. */
		if (mesh.HasTangents)
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
			GenerateTask *task = new GenerateTask(data, mesh, interfaces, writeLock, writer);
			tasks.emplace_back(task);
			args.Scheduluer->Spawn(*task);
		}
		else Log::Warning("Unable to generate tangents for mesh '%s' (mesh doesn't have normals, texture coordinates, or isn't a triangle list)!", name.c_str());
	}

	/* Wait for all of the tasks to complete. */
	bool working;
	do
	{
		working = false;

		for (GenerateTask *cur : tasks)
		{
			if (cur->IsWorking()) working = true;
			else
			{
				/* Wait for a small moment to allow the thread to fully finish, it's safe to delete it already, but might throw a warning. */
				PuThread::Sleep(100);
				working = !tasks.empty();

				/* Delete the task to free up memory once it's completed. */
				tasks.remove(cur);
				delete cur;
				break;
			}
		}

		PuThread::Sleep(100);
	} while (working);

	/* Override the old data. */
	writeLock.lock();
	data.Data = std::move(writer);
	writeLock.unlock();

	Log::Message("Finished generating tangents for model '%s', took %f seconds.", args.DisplayName.c_str(), sw.SecondsAccurate());
	return result;
}