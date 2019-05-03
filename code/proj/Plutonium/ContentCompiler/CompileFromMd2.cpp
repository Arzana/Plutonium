#include "CompileFromMd2.h"
#include "CommandLineArguments.h"
#include <Streams/FileReader.h>
#include <regex>

using namespace Pu;

/* Defines the normals that can be used with MD2. */
Vector3 anorms[162]
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

/* Defines the total play time and if it should loop for common animations. */
std::map<string, std::pair<float, bool>> animations =
{
	{ "stand", std::make_pair(39.0f, true) },
	{ "run", std::make_pair(5.0f, true) },
	{ "attack", std::make_pair(7.0f, false) },
	{ "pain1", std::make_pair(3.0f, false) },
	{ "pain2", std::make_pair(3.0f, false) },
	{ "pain3", std::make_pair(3.0f, false) },
	{ "jump", std::make_pair(5.0f, false) },
	{ "flip", std::make_pair(11.0f, false) },
	{ "salute", std::make_pair(10.0f, false) },
	{ "taunt", std::make_pair(16.0f, false) },
	{ "wave", std::make_pair(10.0f, false) },
	{ "point", std::make_pair(11.0f, false) },
	{ "crstnd", std::make_pair(18.0f, true) },
	{ "crwalk", std::make_pair(5.0f, true) },
	{ "crattack", std::make_pair(8.0f, false) },
	{ "crpain", std::make_pair(3.0f, false) },
	{ "crdeath", std::make_pair(4.0f, false) },
	{ "death1", std::make_pair(5.0f, false) },
	{ "death2", std::make_pair(5.0f, false) },
	{ "death3", std::make_pair(7.0f, false) }
};

struct md2_header_t
{
	int magicNum;
	int version;

	int skinwidth;
	int skinheight;

	int framesize;

	int num_skins;
	int num_vertices;
	int num_st;
	int num_tris;
	int num_glcmds;
	int num_frames;

	int offset_skins;
	int offset_st;
	int offset_tris;
	int offset_frames;
	int offset_glcmds;
	int offset_end;
};

const md2_header_t readMd2Header(FileReader &reader)
{
	/* Attempt to read the file header. */
	md2_header_t header;
	if (!reader.Read(header, 0)) Log::Fatal("Unable to read MD2 file header");

	/* Check if the file is valid. */
	if (header.magicNum != 844121161) Log::Fatal("File is not a valid MD2 file!");
	if (header.version != 8) Log::Fatal("File is not a valid MD2 file!");

	return header;
}

void readMd2TextureNames(FileReader &reader, const md2_header_t &header, Md2LoaderResult &result)
{
	/* Move to the correct reading position. */
	reader.Seek(SeekOrigin::Begin, header.offset_skins);

	/* Read texture names individually. Note that the name is always stored as a 64 byte value. */
	char buffer[64];
	for (size_t i = 0; i < header.num_skins; i++)
	{
		/* Throw if we cannot read the full name. */
		if (reader.Read(reinterpret_cast<byte*>(buffer), 0, sizeof(buffer)) == sizeof(buffer))
		{
			/* Add a copy of the texture name to the textures buffer. */
			result.textures.emplace_back(buffer);
		}
		else Log::Fatal("Unable to read texture name!");
	}
}

void readMd2TextureCoords(FileReader &reader, const md2_header_t &header, Md2LoaderResult &result)
{
	/* Move to the correct reading position. */
	reader.Seek(SeekOrigin::Begin, header.offset_st);

	/* Get the texture size as a vector. */
	const Vector2 textureSize(recip(static_cast<float>(header.skinwidth)), recip(static_cast<float>(header.skinheight)));

	/* Read texture coordinates individually. */
	int16 buffer[2];
	for (size_t i = 0; i < header.num_st; i++)
	{
		/* Throw if we cannot read the full two shorts. */
		if (reader.Read(reinterpret_cast<byte*>(buffer), 0, sizeof(buffer)) == sizeof(buffer))
		{
			/* Convert the specified coordinate to a texture uv. */
			result.texcoords.emplace_back(Vector2(static_cast<float>(buffer[0]), static_cast<float>(buffer[1])) * textureSize);
		}
		else Log::Fatal("Unable to read texture coordinates!");
	}
}

void readMd2Triangles(FileReader &reader, const md2_header_t &header, Md2LoaderResult &result)
{
	/* Move to the correct reading position. */
	reader.Seek(SeekOrigin::Begin, header.offset_tris);

	/* Read triangle indices indices. */
	md2_triangle_t tris;
	for (size_t i = 0; i < header.num_tris; i++)
	{
		/* Throw if we cannot read the full 6 shorts */
		if (reader.Read(tris, 0)) result.shapes.emplace_back(tris);
		else Log::Fatal("Unable to read traingle!");
	}
}

void readMd2Frames(FileReader &reader, const md2_header_t &header, Md2LoaderResult &result)
{
	/* Move to the correct reading position. */
	reader.Seek(SeekOrigin::Begin, header.offset_frames);

	/* Read all individually frames, note that the frame name is always saved as a 16 byte string. */
	char name[16];
	byte vertex[4];
	for (size_t i = 0; i < header.num_frames; i++)
	{
		md2_frame_t frame;

		/* Attempt to read the frame scale. */
		if (!reader.Read(frame.scale, 0)) Log::Fatal("Unable to read scale of frame!");

		/* Attempt to read the frame translation. */
		if (!reader.Read(frame.translation, 0)) Log::Fatal("Unable to read translation of frame!");

		/* Attempt to read the frame name. */
		if (reader.Read(reinterpret_cast<byte*>(name), 0, sizeof(name)) != sizeof(name)) Log::Fatal("Unable to read name of frame!");
		frame.name = name;

		/* Read vertex blob. */
		for (size_t j = 0; j < header.num_vertices; j++)
		{
			/* Throw if we cannot read the vertex. */
			if (reader.Read(vertex, 0, sizeof(vertex)) == sizeof(vertex))
			{
				md2_vertex_t v;

				/* Parse the position of the vertex. */
				v.position = Vector3(static_cast<float>(vertex[0]), static_cast<float>(vertex[1]), static_cast<float>(vertex[2]));

				/* Parse the normal of the vertex. */
				if (vertex[3] > sizeof(anorms) / sizeof(float)) Log::Fatal("Attempting to use unknown normal!");
				v.normal = anorms[vertex[3]];

				/* Add vertex to the blob. */
				frame.vertices.emplace_back(v);
			}
			else Log::Fatal("Unable to read vertex at position!");
		}

		/* Add frame to the result. */
		result.frames.emplace_back(frame);
	}
}

int LoadMd2(const Pu::string & path, Md2LoaderResult & result)
{
	FileReader reader(path.toWide());

	/* Check if file is available. */
	if (!reader.IsOpen()) Log::Fatal("Unable to open MD2 file!");

	/* Read and parse the file data. */
	const md2_header_t header = readMd2Header(reader);
	readMd2TextureNames(reader, header, result);
	readMd2TextureCoords(reader, header, result);
	readMd2Triangles(reader, header, result);
	readMd2Frames(reader, header, result);
	// We don't handle OpenGL command so just ignore them.

	return EXIT_SUCCESS;
}

void Md2ToPum(const Md2LoaderResult & input, PumIntermediate & result)
{
	static std::regex regex("([a-zA-Z]+[0-9]*?)([0-9]{1,2})");

	/*
	MD2 doesn't have nodes but will always have one mesh.
	So we just add a single controllable node with a reference to that mesh.
	*/
	pum_node node;
	node.SetMesh(0);
	result.Nodes.emplace_back(node);

	/* Convert the name to UTF-32 and enable linear for the filters. */
	for (const string &path : input.textures)
	{
		pum_texture texture;
		texture.Identifier = path.toUTF32();
		texture.UsesLinearMagnification = true;
		texture.UsesLinaerMinification = true;
		texture.UsesLinearMipmapMode = true;
		result.Textures.emplace_back(texture);

		/* Just add a new material for every defined texture (they're often not defined). */
		pum_material material;
		material.DiffuseTexture = static_cast<uint32>(result.Textures.size() - 1);
		result.Materials.emplace_back(material);
	}

	/*
	MD2 has baked morph animations, they have to be uncompressed to work with the .pum format.
	This makes the output file larger, but no baking is required anymore as they're already baked.
	*/
	for (const md2_frame_t &frame : input.frames)
	{
		Log::Verbose("Processing frame '%s'.", frame.name.c_str());
		/* Resize for every frame, this saves a lot of time. */
		result.Data.EnsureCapacity(input.shapes.size() * 3 * (sizeof(Vector3) * 2 + sizeof(Vector2)));

		/* Get the name and the frame index from the animation. */
		std::smatch matches;
		std::regex_match(frame.name, matches, regex);
		const ustring animationName = string(matches[1].str()).toUTF32();
		const size_t idx = std::stoull(matches[2].str());

		/* Check if we already have this animation in the result list; if so just add the frame to it. */
		vector<pum_animation>::iterator it = result.Animations.iteratorOf([animationName](const pum_animation &cur) { return cur.Identifier == animationName; });
		if (it != result.Animations.end())
		{	// Currently we assume that they're in the correct order already.... don't know if that's a good idea.
			it->Frames.emplace_back(result.Geometry.size());
		}
		else
		{
			pum_animation animation;
			animation.Identifier = std::move(animationName);
			animation.InterpolationMode = 1;	//Linear
			animation.Type = 1;					// Always morph

			decltype(animations)::const_iterator it = animations.find(animationName.toUTF8());
			if (it != animations.end())
			{
				animation.Duration = it->second.first;
				animation.Looping = it->second.second;
			}
			else animation.Duration = 10.0f;	// Default to 10 seconds.

			animation.Frames.emplace_back(result.Geometry.size());
			result.Animations.emplace_back(std::move(animation));
		}

		/* MD2 frames always have normals and texture coordinates but nothing more. */
		pum_mesh mesh;
		mesh.Identifier = frame.name.toUTF32();
		mesh.Topology = 3;	// triangles.
		mesh.HasNormals = true;
		mesh.HasTextureUvs = true;
		mesh.VertexViewStart = static_cast<uint32>(result.Data.GetSize());

		/* Loop through all triangles in the model. */
		bool firstVrtx = true;
		for (const md2_triangle_t &tris : input.shapes)
		{
			for (size_t i = 0; i < 3; i++)
			{
				/* Gets the vertex and calculate the final position. */
				const md2_vertex_t &vrtx = frame.vertices[tris.vertex_indices[i]];
				const Vector3 pos = frame.scale * vrtx.position + frame.translation;

				/* Merge the point into the bounding box . */
				if (firstVrtx)
				{
					firstVrtx = false;
					mesh.Bounds.LowerBound = pos;
					mesh.Bounds.UpperBound = pos;
				}
				else mesh.Bounds = mesh.Bounds.Merge(pos);

				/* Write the vertex to the buffer. */
				result.Data.Write(pos);
				result.Data.Write(vrtx.normal);
				result.Data.Write(input.texcoords[tris.texture_indices[i]]);
			}
		}

		/* Set the size of the mesh in bytes and add it to our result. */
		mesh.VertexViewSize = static_cast<uint32>(result.Data.GetSize() - mesh.VertexViewStart);
		result.Geometry.emplace_back(mesh);
	}
}