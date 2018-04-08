#include "Graphics\Models\Md2Loader.h"
#include "Streams\FileReader.h"
#include "Streams\FileUtils.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Core\Math\Basics.h"

using namespace Plutonium;

/* Defines the normals that can be used with MD2. */
float anorms[162][3]
{
	{ -0.525731f,  0.000000f,  0.850651f },
	{ -0.442863f,  0.238856f,  0.864188f },
	{ -0.295242f,  0.000000f,  0.955423f },
	{ -0.309017f,  0.500000f,  0.809017f },
	{ -0.162460f,  0.262866f,  0.951056f },
	{ 0.000000f,  0.000000f,  1.000000f },
	{ 0.000000f,  0.850651f,  0.525731f },
	{ -0.147621f,  0.716567f,  0.681718f },
	{ 0.147621f,  0.716567f,  0.681718f },
	{ 0.000000f,  0.525731f,  0.850651f },
	{ 0.309017f,  0.500000f,  0.809017f },
	{ 0.525731f,  0.000000f,  0.850651f },
	{ 0.295242f,  0.000000f,  0.955423f },
	{ 0.442863f,  0.238856f,  0.864188f },
	{ 0.162460f,  0.262866f,  0.951056f },
	{ -0.681718f,  0.147621f,  0.716567f },
	{ -0.809017f,  0.309017f,  0.500000f },
	{ -0.587785f,  0.425325f,  0.688191f },
	{ -0.850651f,  0.525731f,  0.000000f },
	{ -0.864188f,  0.442863f,  0.238856f },
	{ -0.716567f,  0.681718f,  0.147621f },
	{ -0.688191f,  0.587785f,  0.425325f },
	{ -0.500000f,  0.809017f,  0.309017f },
	{ -0.238856f,  0.864188f,  0.442863f },
	{ -0.425325f,  0.688191f,  0.587785f },
	{ -0.716567f,  0.681718f, -0.147621f },
	{ -0.500000f,  0.809017f, -0.309017f },
	{ -0.525731f,  0.850651f,  0.000000f },
	{ 0.000000f,  0.850651f, -0.525731f },
	{ -0.238856f,  0.864188f, -0.442863f },
	{ 0.000000f,  0.955423f, -0.295242f },
	{ -0.262866f,  0.951056f, -0.162460f },
	{ 0.000000f,  1.000000f,  0.000000f },
	{ 0.000000f,  0.955423f,  0.295242f },
	{ -0.262866f,  0.951056f,  0.162460f },
	{ 0.238856f,  0.864188f,  0.442863f },
	{ 0.262866f,  0.951056f,  0.162460f },
	{ 0.500000f,  0.809017f,  0.309017f },
	{ 0.238856f,  0.864188f, -0.442863f },
	{ 0.262866f,  0.951056f, -0.162460f },
	{ 0.500000f,  0.809017f, -0.309017f },
	{ 0.850651f,  0.525731f,  0.000000f },
	{ 0.716567f,  0.681718f,  0.147621f },
	{ 0.716567f,  0.681718f, -0.147621f },
	{ 0.525731f,  0.850651f,  0.000000f },
	{ 0.425325f,  0.688191f,  0.587785f },
	{ 0.864188f,  0.442863f,  0.238856f },
	{ 0.688191f,  0.587785f,  0.425325f },
	{ 0.809017f,  0.309017f,  0.500000f },
	{ 0.681718f,  0.147621f,  0.716567f },
	{ 0.587785f,  0.425325f,  0.688191f },
	{ 0.955423f,  0.295242f,  0.000000f },
	{ 1.000000f,  0.000000f,  0.000000f },
	{ 0.951056f,  0.162460f,  0.262866f },
	{ 0.850651f, -0.525731f,  0.000000f },
	{ 0.955423f, -0.295242f,  0.000000f },
	{ 0.864188f, -0.442863f,  0.238856f },
	{ 0.951056f, -0.162460f,  0.262866f },
	{ 0.809017f, -0.309017f,  0.500000f },
	{ 0.681718f, -0.147621f,  0.716567f },
	{ 0.850651f,  0.000000f,  0.525731f },
	{ 0.864188f,  0.442863f, -0.238856f },
	{ 0.809017f,  0.309017f, -0.500000f },
	{ 0.951056f,  0.162460f, -0.262866f },
	{ 0.525731f,  0.000000f, -0.850651f },
	{ 0.681718f,  0.147621f, -0.716567f },
	{ 0.681718f, -0.147621f, -0.716567f },
	{ 0.850651f,  0.000000f, -0.525731f },
	{ 0.809017f, -0.309017f, -0.500000f },
	{ 0.864188f, -0.442863f, -0.238856f },
	{ 0.951056f, -0.162460f, -0.262866f },
	{ 0.147621f,  0.716567f, -0.681718f },
	{ 0.309017f,  0.500000f, -0.809017f },
	{ 0.425325f,  0.688191f, -0.587785f },
	{ 0.442863f,  0.238856f, -0.864188f },
	{ 0.587785f,  0.425325f, -0.688191f },
	{ 0.688191f,  0.587785f, -0.425325f },
	{ -0.147621f,  0.716567f, -0.681718f },
	{ -0.309017f,  0.500000f, -0.809017f },
	{ 0.000000f,  0.525731f, -0.850651f },
	{ -0.525731f,  0.000000f, -0.850651f },
	{ -0.442863f,  0.238856f, -0.864188f },
	{ -0.295242f,  0.000000f, -0.955423f },
	{ -0.162460f,  0.262866f, -0.951056f },
	{ 0.000000f,  0.000000f, -1.000000f },
	{ 0.295242f,  0.000000f, -0.955423f },
	{ 0.162460f,  0.262866f, -0.951056f },
	{ -0.442863f, -0.238856f, -0.864188f },
	{ -0.309017f, -0.500000f, -0.809017f },
	{ -0.162460f, -0.262866f, -0.951056f },
	{ 0.000000f, -0.850651f, -0.525731f },
	{ -0.147621f, -0.716567f, -0.681718f },
	{ 0.147621f, -0.716567f, -0.681718f },
	{ 0.000000f, -0.525731f, -0.850651f },
	{ 0.309017f, -0.500000f, -0.809017f },
	{ 0.442863f, -0.238856f, -0.864188f },
	{ 0.162460f, -0.262866f, -0.951056f },
	{ 0.238856f, -0.864188f, -0.442863f },
	{ 0.500000f, -0.809017f, -0.309017f },
	{ 0.425325f, -0.688191f, -0.587785f },
	{ 0.716567f, -0.681718f, -0.147621f },
	{ 0.688191f, -0.587785f, -0.425325f },
	{ 0.587785f, -0.425325f, -0.688191f },
	{ 0.000000f, -0.955423f, -0.295242f },
	{ 0.000000f, -1.000000f,  0.000000f },
	{ 0.262866f, -0.951056f, -0.162460f },
	{ 0.000000f, -0.850651f,  0.525731f },
	{ 0.000000f, -0.955423f,  0.295242f },
	{ 0.238856f, -0.864188f,  0.442863f },
	{ 0.262866f, -0.951056f,  0.162460f },
	{ 0.500000f, -0.809017f,  0.309017f },
	{ 0.716567f, -0.681718f,  0.147621f },
	{ 0.525731f, -0.850651f,  0.000000f },
	{ -0.238856f, -0.864188f, -0.442863f },
	{ -0.500000f, -0.809017f, -0.309017f },
	{ -0.262866f, -0.951056f, -0.162460f },
	{ -0.850651f, -0.525731f,  0.000000f },
	{ -0.716567f, -0.681718f, -0.147621f },
	{ -0.716567f, -0.681718f,  0.147621f },
	{ -0.525731f, -0.850651f,  0.000000f },
	{ -0.500000f, -0.809017f,  0.309017f },
	{ -0.238856f, -0.864188f,  0.442863f },
	{ -0.262866f, -0.951056f,  0.162460f },
	{ -0.864188f, -0.442863f,  0.238856f },
	{ -0.809017f, -0.309017f,  0.500000f },
	{ -0.688191f, -0.587785f,  0.425325f },
	{ -0.681718f, -0.147621f,  0.716567f },
	{ -0.442863f, -0.238856f,  0.864188f },
	{ -0.587785f, -0.425325f,  0.688191f },
	{ -0.309017f, -0.500000f,  0.809017f },
	{ -0.147621f, -0.716567f,  0.681718f },
	{ -0.425325f, -0.688191f,  0.587785f },
	{ -0.162460f, -0.262866f,  0.951056f },
	{ 0.442863f, -0.238856f,  0.864188f },
	{ 0.162460f, -0.262866f,  0.951056f },
	{ 0.309017f, -0.500000f,  0.809017f },
	{ 0.147621f, -0.716567f,  0.681718f },
	{ 0.000000f, -0.525731f,  0.850651f },
	{ 0.425325f, -0.688191f,  0.587785f },
	{ 0.587785f, -0.425325f,  0.688191f },
	{ 0.688191f, -0.587785f,  0.425325f },
	{ -0.955423f,  0.295242f,  0.000000f },
	{ -0.951056f,  0.162460f,  0.262866f },
	{ -1.000000f,  0.000000f,  0.000000f },
	{ -0.850651f,  0.000000f,  0.525731f },
	{ -0.955423f, -0.295242f,  0.000000f },
	{ -0.951056f, -0.162460f,  0.262866f },
	{ -0.864188f,  0.442863f, -0.238856f },
	{ -0.951056f,  0.162460f, -0.262866f },
	{ -0.809017f,  0.309017f, -0.500000f },
	{ -0.864188f, -0.442863f, -0.238856f },
	{ -0.951056f, -0.162460f, -0.262866f },
	{ -0.809017f, -0.309017f, -0.500000f },
	{ -0.681718f,  0.147621f, -0.716567f },
	{ -0.681718f, -0.147621f, -0.716567f },
	{ -0.850651f,  0.000000f, -0.525731f },
	{ -0.688191f,  0.587785f, -0.425325f },
	{ -0.587785f,  0.425325f, -0.688191f },
	{ -0.425325f,  0.688191f, -0.587785f },
	{ -0.425325f, -0.688191f, -0.587785f },
	{ -0.587785f, -0.425325f, -0.688191f },
	{ -0.688191f, -0.587785f, -0.425325f }
};

/* Defines the header used within the MD2 file. */
struct md2_header_t
{
	/* Magic number is used to indentify if the file is an MD2 file, value should be 'IDP2'. */
	int magicNum;
	/* The version of MD2 used, this should always be 8. */
	int version;

	/* The width of the texture that can be used with this model. */
	int skinwidth;
	/* The height of the texture that can be used with this model. */
	int skinheight;

	/* The size (in bytes) of an animation frame (not used by default). */
	int framesize;

	/* The amount of skins predefined in the file (can be zero). */
	int num_skins;
	/* The amount of vertices per frame blob. */
	int num_vertices;
	/* The number of texture coordinates per frame blob. */
	int num_st;
	/* The number of triangles defined by the model. */
	int num_tris;
	/* The number of OpenGL commands specified in the file (not used by default). */
	int num_glcmds;
	/* The total number of animation frames specified by the MD2 file. */
	int num_frames;

	/* The file seek offset for the texture names. */
	int offset_skins;
	/* The file seek offset for the texture coordinates. */
	int offset_st;
	/* The file seek offset for the triangles. */
	int offset_tris;
	/* The file seek offset for the animation frames. */
	int offset_frames;
	/* The file seek offset for the OpenGL commands. */
	int offset_glcmds;
	/* The file seek offset for the end of the file (not used by default). */
	int offset_end;
};

const md2_header_t _CrtReadMd2Header(FileReader *reader)
{
	/* Attempt to read the file header. */
	md2_header_t header;
	if (reader->Read(reinterpret_cast<byte*>(&header), 0, sizeof(md2_header_t)) != sizeof(md2_header_t))
	{
		LOG_THROW("Unable to read MD2 file header!");
	}

	/* Check if the file is valid. */
	LOG_THROW_IF(header.magicNum != _CrtGetMagicNum("IDP2"), "File '%s' is not a valid MD2 file!", reader->GetFileName());
	LOG_THROW_IF(header.version != 8, "File '%s' is not a valid MD2 file!", reader->GetFileName());

	return header;
}

void _CrtReadMd2TextureNames(FileReader *reader, const md2_header_t *header, Md2LoaderResult *result)
{
	/* Move to the correct reading position. */
	reader->Seek(SeekOrigin::Begin, header->offset_skins);

	/* Read texture names individually. Note that the name is always stored as a 64 byte value. */
	char buffer[64];
	for (size_t i = 0; i < header->num_skins; i++)
	{
		/* Throw if we cannot read the full name. */
		if (reader->Read(reinterpret_cast<byte*>(buffer), 0, sizeof(buffer)) == sizeof(buffer))
		{
			/* Add a copy of the texture name to the textures buffer. */
			result->textures.push_back(heapstr(buffer));
		}
		else LOG_THROW("Unable to read texture name at position %zu!", i);
	}
}

void _CrtReadMd2TextureCoords(FileReader *reader, const md2_header_t *header, Md2LoaderResult *result)
{
	/* Move to the correct reading position. */
	reader->Seek(SeekOrigin::Begin, header->offset_st);

	/* Get the texture size as a vector. */
	Vector2 textureSize = Vector2(recip(static_cast<float>(header->skinwidth)), recip(static_cast<float>(header->skinheight)));

	/* Read texture coordinates individually. */
	int16 buffer[2];
	for (size_t i = 0; i < header->num_st; i++)
	{
		/* Throw if we cannot read the full two shorts. */
		if (reader->Read(reinterpret_cast<byte*>(buffer), 0, sizeof(buffer)) == sizeof(buffer))
		{
			/* Convert the specified coordinate to a texture uv. */
			Vector2 uv = Vector2(static_cast<float>(buffer[0]), static_cast<float>(buffer[1])) * textureSize;
			result->texcoords.push_back(uv);
		}
		else LOG_THROW("Unable to read texture coordinates at position %zu!", i);
	}
}

void _CrtReadMd2Triangles(FileReader *reader, const md2_header_t *header, Md2LoaderResult *result)
{
	/* Move to the correct reading position. */
	reader->Seek(SeekOrigin::Begin, header->offset_tris);

	/* Read triangle indices indices. */
	md2_triangle_t buffer;
	for (size_t i = 0; i < header->num_tris; i++)
	{
		/* Throw if we cannot read the full 6 shorts */
		if (reader->Read(reinterpret_cast<byte*>(&buffer), 0, sizeof(buffer)) == sizeof(buffer))
		{
			result->shapes.push_back(buffer);
		}
		else LOG_THROW("Unable to read traingle at position %zu!", i);
	}
}

void _CrtReadMd2OpenGLCommands(FileReader *reader, const md2_header_t *header, Md2LoaderResult *result)
{
	/* Move to the correct reading position. */
	reader->Seek(SeekOrigin::Begin, header->offset_glcmds);

	/* Read OpenGL commands. */
	int32 cmd;
	for (size_t i = 0; i < header->num_glcmds; i++)
	{
		/* Throw if we cannot read the full int. */
		if (reader->Read(reinterpret_cast<byte*>(&cmd), 0, sizeof(cmd)) == sizeof(cmd))
		{
			result->OpenGLCommands.push_back(cmd);
		}
		else LOG_THROW("Unable to read OpenGL command at position %zu!", i);
	}
}

void _CrtReadMd2Frames(FileReader *reader, const md2_header_t *header, Md2LoaderResult *result)
{
	/* Move to the correct reading position. */
	reader->Seek(SeekOrigin::Begin, header->offset_frames);

	/* Read all individually frames, note that the frame name is always saved as a 16 byte string. */
	char name[16];
	byte vertex[4];
	for (size_t i = 0; i < header->num_frames; i++)
	{
		md2_frame_t frame;

		/* Attempt to read the frame scale. */
		if (reader->Read(reinterpret_cast<byte*>(&frame.scale), 0, sizeof(Vector3)) != sizeof(Vector3)) LOG_THROW("Unable to read scale of frame at position %zu!", i);

		/* Attempt to read the frame translation. */
		if (reader->Read(reinterpret_cast<byte*>(&frame.translation), 0, sizeof(Vector3)) != sizeof(Vector3)) LOG_THROW("Unable to read translation of frame at position %zu!", i);

		/* Attempt to read the frame name. */
		if (reader->Read(reinterpret_cast<byte*>(name), 0, sizeof(name)) != sizeof(name)) LOG_THROW("Unable to read name of frame at position %zu!", i);
		frame.name = Plutonium::heapstr(name);

		/* Read vertex blob. */
		for (size_t j = 0; j < header->num_vertices; j++)
		{
			/* Throw if we cannot read the vertex. */
			if (reader->Read(vertex, 0, sizeof(vertex)) == sizeof(vertex))
			{
				md2_vertex_t v;

				/* Parse the position of the vertex. */
				v.position = Vector3(static_cast<float>(vertex[0]), static_cast<float>(vertex[1]), static_cast<float>(vertex[2]));

				/* Parse the normal of the vertex. */
				LOG_THROW_IF(vertex[3] > sizeof(anorms) / sizeof(float), "Attempting to use unknown normal at position %zu in frame '%s'!", j, frame.name);
				float *n = anorms[vertex[3]];
				v.normal = Vector3(n[0], n[1], n[2]);

				/* Add vertex to the blob. */
				frame.vertices.push_back(v);
			}
			else LOG_THROW("Unable to read vertex at position %zu in frame '%s'!", j, frame.name);
		}

		/* Add frame to the result. */
		result->frames.push_back(frame);
	}
}

const Plutonium::Md2LoaderResult * Plutonium::_CrtLoadMd2(const char * path)
{
	/* Create result and initialize file reader. */
	Md2LoaderResult *result = new Md2LoaderResult();
	FileReader reader(path);

	/* Check if file is available. */
	LOG_THROW_IF(!reader.IsOpen(), "Unable to open MD2 file: '%s'!", reader.GetFileName());

	/* Read and parse the file data. */
	const md2_header_t header = _CrtReadMd2Header(&reader);
	_CrtReadMd2TextureNames(&reader, &header, result);
	_CrtReadMd2TextureCoords(&reader, &header, result);
	_CrtReadMd2Triangles(&reader, &header, result);
	_CrtReadMd2OpenGLCommands(&reader, &header, result);
	_CrtReadMd2Frames(&reader, &header, result);

	return result;
}

Plutonium::Md2LoaderResult::~Md2LoaderResult(void)
{
	/* Make sure we remove the heap refrence. */
	for (size_t i = 0; i < textures.size(); i++) free_s(textures.at(i));
}