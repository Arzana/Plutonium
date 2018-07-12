#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Streams\StringParsing.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Core\EnumUtils.h"

/*
Syoyo Fujita.
tinyobjloader (2012)
https://github.com/syoyo/tinyobjloader
*/

using namespace Plutonium;
using namespace std;

void LoadMaterialLibraryFromFile(const char*, const char*, ObjLoaderResult*);

#pragma region Constructors
Plutonium::ObjLoaderVertex::ObjLoaderVertex(void)
	: Vertex(-1), Normal(-1), TexCoord(-1)
{}

Plutonium::ObjLoaderMesh::ObjLoaderMesh(void)
	: Name(""), Indices(), Material(-1), SmoothingGroups()
{}

Plutonium::ObjLoaderMesh::ObjLoaderMesh(const ObjLoaderMesh & value)
	: Name(""), Indices(), Material(value.Material), SmoothingGroups()
{
	if (strlen(value.Name) > 0) Name = heapstr(value.Name);

	Indices = std::vector<ObjLoaderVertex>(value.Indices);
	SmoothingGroups = std::vector<uint64>(value.SmoothingGroups);
}

Plutonium::ObjLoaderMesh::ObjLoaderMesh(ObjLoaderMesh && value)
{
	Name = value.Name;
	Indices = std::move(value.Indices);
	Material = value.Material;
	SmoothingGroups = std::move(value.SmoothingGroups);

	value.Name = "";
	value.Indices = std::vector<ObjLoaderVertex>();
	value.Material = -1;
	value.SmoothingGroups = std::vector<uint64>();
}

Plutonium::ObjLoaderMesh::~ObjLoaderMesh(void)
{
	if (strlen(Name) > 0) free_s(Name);
}

ObjLoaderMesh & Plutonium::ObjLoaderMesh::operator=(const ObjLoaderMesh & other)
{
	if (this != &other)
	{
		/* Release old data. */
		if (strlen(Name) > 0) free_s(Name);

		/* Copy over data. */
		if (strlen(other.Name)) Name = heapstr(other.Name);
		Indices = std::vector<ObjLoaderVertex>(other.Indices);
		Material = other.Material;
		SmoothingGroups = std::vector<uint64>(other.SmoothingGroups);
	}

	return *this;
}

ObjLoaderMesh & Plutonium::ObjLoaderMesh::operator=(ObjLoaderMesh && other)
{
	if (this != &other)
	{
		/* Release old data. */
		if (strlen(Name) > 0) free_s(Name);

		/* Move over data. */
		Name = other.Name;
		Indices = std::move(other.Indices);
		Material = other.Material;
		SmoothingGroups = std::move(other.SmoothingGroups);

		/* Clear moved data. */
		other.Name = "";
		other.Indices = std::vector<ObjLoaderVertex>();
		other.Material = -1;
		other.SmoothingGroups = std::vector<uint64>();
	}

	return *this;
}

Plutonium::ObjLoaderTextureMap::ObjLoaderTextureMap(bool isBump)
	: Path(""), Type(ObjLoaderMapType::None),
	Sharpness(1.0f), Brightness(0.0f), Contrast(1.0f),
	Origin(), Scale(1.0f), Turbulence(), ClampedCoords(false),
	BlendH(true), BlendV(true),
	ScalarOrBumpChannel(isBump ? ObjLoaderChannel::Luminance : ObjLoaderChannel::Matte), BumpMod(1.0f)
{}

Plutonium::ObjLoaderTextureMap::ObjLoaderTextureMap(const ObjLoaderTextureMap & value)
	: Path(""), Type(value.Type),
	Sharpness(value.Sharpness), Brightness(value.Brightness), Contrast(value.Contrast),
	Origin(value.Origin), Scale(value.Scale), Turbulence(value.Turbulence), ClampedCoords(value.ClampedCoords),
	BlendH(value.BlendH), BlendV(value.BlendV), ScalarOrBumpChannel(value.ScalarOrBumpChannel), BumpMod(value.BumpMod)
{
	if (strlen(value.Path) > 0) Path = heapstr(value.Path);
}

Plutonium::ObjLoaderTextureMap::ObjLoaderTextureMap(ObjLoaderTextureMap && value)
{
	Path = value.Path;
	Type = value.Type;
	Sharpness = value.Sharpness;
	Brightness = value.Brightness;
	Contrast = value.Contrast;
	Origin = value.Origin;
	Scale = value.Scale;
	Turbulence = value.Turbulence;
	ClampedCoords = value.ClampedCoords;
	BlendH = value.BlendH;
	BlendV = value.BlendV;
	ScalarOrBumpChannel = value.ScalarOrBumpChannel;
	BumpMod = value.BumpMod;

	value.Path = "";
	value.Type = ObjLoaderMapType::None;
	value.Sharpness = 1.0f;
	value.Brightness = 0.0f;
	value.Contrast = 1.0f;
	value.Origin = Vector3::Zero;
	value.Scale = Vector3::One;
	value.ClampedCoords = false;
	value.BlendH = true;
	value.BlendV = true;
	value.ScalarOrBumpChannel = ObjLoaderChannel::Matte;
	value.BumpMod = 1.0f;
}

Plutonium::ObjLoaderTextureMap::~ObjLoaderTextureMap(void)
{
	if (strlen(Path) > 0) free_s(Path);
}

ObjLoaderTextureMap & Plutonium::ObjLoaderTextureMap::operator=(const ObjLoaderTextureMap & other)
{
	if (this != &other)
	{
		/* Release old data. */
		if (strlen(Path) > 0) free_s(Path);

		/* Copy over new data. */
		Path = strlen(other.Path) > 0 ? heapstr(other.Path) : "";
		Type = other.Type;
		Sharpness = other.Sharpness;
		Brightness = other.Brightness;
		Contrast = other.Contrast;
		Origin = other.Origin;
		Scale = other.Scale;
		Turbulence = other.Turbulence;
		ClampedCoords = other.ClampedCoords;
		BlendH = other.BlendH;
		BlendV = other.BlendV;
		ScalarOrBumpChannel = other.ScalarOrBumpChannel;
		BumpMod = other.BumpMod;
	}

	return *this;
}

ObjLoaderTextureMap & Plutonium::ObjLoaderTextureMap::operator=(ObjLoaderTextureMap && other)
{
	if (this != &other)
	{
		/* Release old data. */
		if (strlen(Path) > 0) free_s(Path);

		/* Move over data. */
		Path = other.Path;
		Type = other.Type;
		Sharpness = other.Sharpness;
		Brightness = other.Brightness;
		Contrast = other.Contrast;
		Origin = other.Origin;
		Scale = other.Scale;
		Turbulence = other.Turbulence;
		ClampedCoords = other.ClampedCoords;
		BlendH = other.BlendH;
		BlendV = other.BlendV;
		ScalarOrBumpChannel = other.ScalarOrBumpChannel;
		BumpMod = other.BumpMod;

		/* Reset old data. */
		other.Path = "";
		other.Type = ObjLoaderMapType::None;
		other.Sharpness = 1.0f;
		other.Brightness = 0.0f;
		other.Contrast = 1.0f;
		other.Origin = Vector3::Zero;
		other.Scale = Vector3::One;
		other.ClampedCoords = false;
		other.BlendH = true;
		other.BlendV = true;
		other.ScalarOrBumpChannel = ObjLoaderChannel::Matte;
		other.BumpMod = 1.0f;
	}

	return *this;
}

bool Plutonium::ObjLoaderTextureMap::operator!=(const ObjLoaderTextureMap & other) const
{
	if (eqlstr(Path, other.Path)) return false;
	if (Type == other.Type) return false;
	if (Sharpness == other.Sharpness) return false;
	if (Brightness == other.Brightness) return false;
	if (Contrast == other.Contrast) return false;
	if (Origin == other.Origin) return false;
	if (Scale == other.Scale) return false;
	if (Turbulence == other.Turbulence) return false;
	if (ClampedCoords == other.ClampedCoords) return false;
	if (BlendH == other.BlendH) return false;
	if (BlendV == other.BlendV) return false;
	if (ScalarOrBumpChannel == ScalarOrBumpChannel) return false;
	return BumpMod != other.BumpMod;
}

Plutonium::ObjLoaderMaterial::ObjLoaderMaterial(void)
	: Name(""), IlluminationModel(0),
	Ambient(Color::Black), Diffuse(Color::Black), Specular(Color::Black),
	Transmittance(Color::White), HighlightExponent(1.0f), OpticalDensity(1.0f), Dissolve(1.0f),
	AmbientMap(false), DiffuseMap(false), SpecularMap(false), HighlightMap(false),
	BumpMap(true), DisplacementMap(false), AlphaMap(false), ReflectionMap(false)
{}

Plutonium::ObjLoaderMaterial::ObjLoaderMaterial(const ObjLoaderMaterial & value)
	: Name(""), IlluminationModel(value.IlluminationModel),
	Ambient(value.Ambient), Diffuse(value.Diffuse), Specular(value.Specular),
	Transmittance(value.Transmittance), HighlightExponent(value.HighlightExponent), OpticalDensity(value.OpticalDensity), Dissolve(value.Dissolve),
	AmbientMap(value.AmbientMap), DiffuseMap(value.DiffuseMap), SpecularMap(value.SpecularMap), HighlightMap(value.HighlightMap),
	BumpMap(value.BumpMap), DisplacementMap(value.DisplacementMap), AlphaMap(value.AlphaMap), ReflectionMap(value.ReflectionMap)
{
	if (strlen(value.Name) > 0) Name = heapstr(value.Name);
}

Plutonium::ObjLoaderMaterial::ObjLoaderMaterial(ObjLoaderMaterial && value)
	: AmbientMap(false), DiffuseMap(false), SpecularMap(false), HighlightMap(false),
	BumpMap(true), DisplacementMap(false), AlphaMap(false), ReflectionMap(false)
{
	/* Move over data. */
	Name = value.Name;
	Ambient = value.Ambient;
	Diffuse = value.Diffuse;
	Specular = value.Specular;
	Transmittance = value.Transmittance;
	HighlightExponent = value.HighlightExponent;
	OpticalDensity = value.OpticalDensity;
	Dissolve = value.Dissolve;
	IlluminationModel = value.IlluminationModel;
	AmbientMap = std::move(value.AmbientMap);
	DiffuseMap = std::move(value.DiffuseMap);
	SpecularMap = std::move(value.SpecularMap);
	HighlightMap = std::move(value.HighlightMap);
	BumpMap = std::move(value.BumpMap);
	DisplacementMap = std::move(value.DisplacementMap);
	AlphaMap = std::move(value.AlphaMap);
	ReflectionMap = std::move(value.ReflectionMap);

	/* Reset old data. */
	value.Name = "";
	value.Ambient = Color::Black;
	value.Diffuse = Color::Black;
	value.Specular = Color::Black;
	value.Transmittance = Color::White;
	value.HighlightExponent = 1.0f;
	value.OpticalDensity = 1.0f;
	value.Dissolve = 1.0f;
	value.IlluminationModel = 0;
}

Plutonium::ObjLoaderMaterial::~ObjLoaderMaterial(void)
{
	if (strlen(Name) > 0) free_s(Name);
}

ObjLoaderMaterial & Plutonium::ObjLoaderMaterial::operator=(const ObjLoaderMaterial & other)
{
	if (this != &other)
	{
		/* Release old data. */
		if (strlen(Name) > 0) free_s(Name);

		/* Copy over new data. */
		Name = strlen(other.Name) > 0 ? heapstr(other.Name) : "";
		Ambient = other.Ambient;
		Diffuse = other.Diffuse;
		Specular = other.Specular;
		Transmittance = other.Transmittance;
		HighlightExponent = other.HighlightExponent;
		OpticalDensity = other.OpticalDensity;
		Dissolve = other.Dissolve;
		IlluminationModel = other.IlluminationModel;
		AmbientMap = other.AmbientMap;
		DiffuseMap = other.DiffuseMap;
		SpecularMap = other.SpecularMap;
		HighlightMap = other.HighlightMap;
		BumpMap = other.BumpMap;
		DisplacementMap = other.DisplacementMap;
		AlphaMap = other.AlphaMap;
		ReflectionMap = other.ReflectionMap;
	}

	return *this;
}

ObjLoaderMaterial & Plutonium::ObjLoaderMaterial::operator=(ObjLoaderMaterial && other)
{
	if (this != &other)
	{
		/* Release old data. */
		if (strlen(Name) > 0) free_s(Name);

		/* Move over new data. */
		Name = other.Name;
		Ambient = other.Ambient;
		Diffuse = other.Diffuse;
		Specular = other.Specular;
		Transmittance = other.Transmittance;
		HighlightExponent = other.HighlightExponent;
		OpticalDensity = other.OpticalDensity;
		Dissolve = other.Dissolve;
		IlluminationModel = other.IlluminationModel;
		AmbientMap = std::move(other.AmbientMap);
		DiffuseMap = std::move(other.DiffuseMap);
		SpecularMap = std::move(other.SpecularMap);
		HighlightMap = std::move(other.HighlightMap);
		BumpMap = std::move(other.BumpMap);
		DisplacementMap = std::move(other.DisplacementMap);
		AlphaMap = std::move(other.AlphaMap);
		ReflectionMap = std::move(other.ReflectionMap);

		/* Reset old data. */
		other.Name = "";
		other.Ambient = Color::Black;
		other.Diffuse = Color::Black;
		other.Specular = Color::Black;
		other.Transmittance = Color::White;
		other.HighlightExponent = 1.0f;
		other.OpticalDensity = 1.0f;
		other.Dissolve = 1.0f;
		other.IlluminationModel = 0;
	}

	return *this;
}

Plutonium::ObjLoaderResult::ObjLoaderResult(void)
	: Vertices(), Normals(), TexCoords(), Shapes(), Materials()
{}
#pragma endregion

#pragma region Checks / moves
/* Gets whether a specified character is a space. */
#define IS_SPACE(x)		((x) == ' ' || (x) == '\t')

/* Gets whether a specified character is a newline. */
#define IS_NEWLINE(x)	((x) == '\n' || (x) == '\r' || (x) == '\0')

/* Skips leading spaces and tabs. */
inline void SkipUseless(const char **line)
{
	while (IS_SPACE(*line[0])) ++(*line);
}

/* Pushes the old mesh to the result and resets the current mesh. */
inline void PushShapeIfNeeded(ObjLoaderResult *result, ObjLoaderMesh *curMesh, bool keepName)
{
	const char *name = heapstr(curMesh->Name);

	/* Check if current mesh has data. */
	if (::strlen(name) > 0)
	{
		/* Push old mesh. */
		result->Shapes.push_back(*curMesh);
		*curMesh = ObjLoaderMesh();

		/* Keep the old name if requested. */
		if (keepName) curMesh->Name = name;
	}
	else free_s(name);
}
#pragma endregion

#pragma region Parsing
bool ParseOnOff(const char **line, bool def = true)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = (*line) + strcspn(*line, " \t\r");

	/* Parse value. */
	bool result = def;
	if (!strncmp(*line, "on", 2)) result = true;
	else if (!strncmp(*line, "off", 3)) result = false;

	/* Increase line position and return result. */
	*line = end;
	return result;
}

ObjLoaderMapType ParseMapType(const char **line, ObjLoaderMapType def = ObjLoaderMapType::None)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = (*line) + strcspn(*line, " \t");

	/* Parse value. */
	ObjLoaderMapType result = def;
	if (!strncmp(*line, "sphere", 6)) result = ObjLoaderMapType::Sphere;
	else if (!strncmp(*line, "cube_left", 9)) result = ObjLoaderMapType::CubeLeft;
	else if (!strncmp(*line, "cube_right", 10)) result = ObjLoaderMapType::CubeRight;
	else if (!strncmp(*line, "cube_front", 10)) result = ObjLoaderMapType::CubeFront;
	else if (!strncmp(*line, "cube_back", 9)) result = ObjLoaderMapType::CubeBack;
	else if (!strncmp(*line, "cube_top", 8)) result = ObjLoaderMapType::CubeTop;
	else if (!strncmp(*line, "cube_bottom", 11)) result = ObjLoaderMapType::CubeBottom;

	/* Increase line position and return result. */
	*line = end;
	return result;
}

ObjLoaderChannel ParseChannel(const char **line, ObjLoaderChannel def = ObjLoaderChannel::None)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = (*line) + strcspn(*line, " \t");

	/* Parse value. */
	ObjLoaderChannel result = def;
	if ((end - *line) == 1) result = _CrtInt2Enum<ObjLoaderChannel>(**line);

	/* Increase line position and return result. */
	*line = end;
	return result;
}

int ParseInt(const char **line)
{
	/* get begin and final read point. */
	SkipUseless(line);
	const char *end = (*line) + strcspn(*line, " \t\r");

	/* Parse value. */
	int result = atoi(*line);

	/* Increase line position and return result. */
	*line = end;
	return result;
}

float ParseFloat(const char **line, float def = 0.0f)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = (*line) + strcspn(*line, " \t\r");

	/* Parse value. */
	float result = def;
	tryParseFloat(*line, end, &result);

	/* Increase line position and return result. */
	*line = end;
	return result;
}

Vector2 ParseFloat2(const char **line, float defX = 0.0f, float defY = 0.0f)
{
	float x = ParseFloat(line, defX);
	float y = ParseFloat(line, defY);
	return Vector2(x, y);
}

Vector3 ParseFloat3(const char **line, float defX = 0.0f, float defY = 0.0f, float defZ = 0.0f)
{
	float x = ParseFloat(line, defX);
	float y = ParseFloat(line, defY);
	float z = ParseFloat(line, defZ);
	return Vector3(x, y, z);
}

Vector4 ParseFloat4(const char **line, float defX = 0.0f, float defY = 0.0f, float defZ = 0.0f, float defW = 0.0f)
{
	float x = ParseFloat(line, defX);
	float y = ParseFloat(line, defY);
	float z = ParseFloat(line, defZ);
	float w = ParseFloat(line, defW);
	return Vector4(x, y, z, w);
}

Color ParseColor3(const char **line, float defR = 0.0f, float defG = 0.0f, float defB = 0.0f)
{
	Vector3 v = ParseFloat3(line, defR, defG, defB);
	return Color(v.X, v.Y, v.Z);
}

bool TryFixIndex(int64 idx, int64 n, int64 *result)
{
	ASSERT_IF(!result, "Cannot pass invalud out parameter!");

	/* If input is positive, convert from normal to zero based. */
	if (idx > 0)
	{
		*result = idx - 1;
		return true;
	}

	/* if input is negative, convert from normal to zero based and invert direction. */
	if (idx < 0)
	{
		*result = n + idx;
		return true;
	}

	/* .obj doesn't allow for zero index values. */
	return false;
}

bool TryParseTriple(const char **line, size_t vSize, size_t vnSize, size_t vtSize, ObjLoaderVertex *result)
{
	ASSERT_IF(!result, "Cannot pass invalid out parameter!");

	/* Parse vertex index and move read position. */
	if (!TryFixIndex(atoi(*line), vSize, &result->Vertex)) return false;
	*line += strcspn(*line, "/ \t\r");

	/* Check if end is reached. */
	if ((*line)[0] != '/') return true;
	++(*line);

	if ((*line)[0] == '/')
	{
		/* Parse normal index and move read position. */
		++(*line);
		if (!TryFixIndex(atoi(*line), vnSize, &result->Normal)) return false;
		*line += strcspn(*line, "/ \t\r");
		return true;
	}

	/* Parse texture coordinate index and move read position. */
	if (!TryFixIndex(atoi(*line), vtSize, &result->TexCoord)) return false;
	*line += strcspn(*line, "/ \t\r");

	/* Check if end is reached. */
	if ((*line)[0] != '/') return true;
	++(*line);

	/* Parse normal index and move read position. */
	if (!TryFixIndex(atoi(*line), vnSize, &result->Normal)) return false;
	*line += strcspn(*line, "/ \t\r");

	return true;
}

void TriangulateQuad(ObjLoaderResult *result, std::vector<ObjLoaderVertex> *vertices)
{
	/* On debug mode check for valid inputs. */
#if defined (DEBUG)
	LOG_THROW_IF(vertices->size() != 4, "Input is not a valid quad, %zu vertices!", vertices->size());
	for (size_t i = 0; i < vertices->size(); i++)
	{
		int64 j = vertices->at(i).Vertex;
		LOG_THROW_IF(j == -1 || static_cast<size_t>(j) >= result->Vertices.size(), "Input vertex index (%d) is not defined!", j);
	}
#endif

	/* Get indices. */
	ObjLoaderVertex v1 = vertices->at(0);
	ObjLoaderVertex v2 = vertices->at(1);
	ObjLoaderVertex v3 = vertices->at(2);
	ObjLoaderVertex v4 = vertices->at(3);

	/* Get positions associated with indices. */
	Vector3 p1 = result->Vertices.at(v1.Vertex);
	Vector3 p2 = result->Vertices.at(v2.Vertex);
	Vector3 p3 = result->Vertices.at(v3.Vertex);
	Vector3 p4 = result->Vertices.at(v4.Vertex);

	/* Clear old values. */
	vertices->clear();
	vertices->reserve(6);

	/* Calculate shortest distance. */
	float d1 = dist(p1, p3);
	float d2 = dist(p2, p4);

	/* Split along b-d axis. */
	if (d1 > d2)
	{
		vertices->push_back(v1);
		vertices->push_back(v2);
		vertices->push_back(v4);

		vertices->push_back(v2);
		vertices->push_back(v3);
		vertices->push_back(v4);
	}
	/* Split along a-c axis. */
	else
	{
		vertices->push_back(v1);
		vertices->push_back(v2);
		vertices->push_back(v3);

		vertices->push_back(v1);
		vertices->push_back(v3);
		vertices->push_back(v4);
	}
}
#pragma endregion

#pragma region Obj Line handling
/* Handles the vertex line. */
inline void HandleVertexLine(const char *line, ObjLoaderResult *result)
{
	result->Vertices.push_back(ParseFloat3(&line));
}

/* Handles the normal line. */
inline void HandleNormalLine(const char *line, ObjLoaderResult *result)
{
	result->Normals.push_back(ParseFloat3(&line));
}

/* Handles the texture coordinate line. */
inline void HandleTexCoordLine(const char *line, ObjLoaderResult *result)
{
	result->TexCoords.push_back(ParseFloat2(&line));
}

/* Handles the face line. */
inline void HandleFaceLine(const char *line, ObjLoaderResult *result, ObjLoaderMesh *curMesh, uint64 smoothingGroup)
{
	/* Skip leading spaces. */
	SkipUseless(&line);

	/* Add current face's smoothing group. */
	curMesh->SmoothingGroups.push_back(smoothingGroup);

	/* Get required buffer sizes. */
	const size_t vSize = result->Vertices.size() / 3;
	const size_t vnSize = result->Normals.size() / 3;
	const size_t vtSize = result->TexCoords.size() / 2;

	/* Read untill no more faces are found. */
	std::vector<ObjLoaderVertex> face;
	while (!IS_NEWLINE(line[0]))
	{
		/* Try parse the value and throw is failed (file corrupt). */
		ObjLoaderVertex vi;
		if (!TryParseTriple(&line, vSize, vnSize, vtSize, &vi)) LOG_THROW("Failed to parse face!\nLINE:		%s", line);

		/* Add face to mesh. */
		face.push_back(vi);
		line += strspn(line, " \t\r");
	}

	/* Check if we can triangulate the shape. */
	LOG_THROW_IF(face.size() < 3 || face.size() > 4, "Cannot convert face with %zu vertices into triangles!", face.size());

	/* If shape is a quad, triangulate it and make sure to add the smoothing group for the new triangle. */
	if (face.size() == 4)
	{
		TriangulateQuad(result, &face);
		curMesh->SmoothingGroups.push_back(smoothingGroup);
	}

	/* Push vertex count to mesh. */
	curMesh->Indices.insert(curMesh->Indices.end(), face.begin(), face.end());
}

/* Handles the use material line. */
inline void HandleUseMaterialLine(const char *line, ObjLoaderResult *result, ObjLoaderMesh *curMesh)
{
	/* Search for the material in the defined list. */
	for (size_t i = 0; i < result->Materials.size(); i++)
	{
		if (eqlstr(result->Materials.at(i).Name, line))
		{
			/* Check if the material is already defined. */
			if (curMesh->Material != -1)
			{
				/* Push the old shape to the result and log the creation of a new shape. */
				LOG_WAR("Redefined material for shape %s (%s -> %s), creating new shape!", curMesh->Name, result->Materials.at(curMesh->Material).Name, line);
				PushShapeIfNeeded(result, curMesh, true);
			}

			/* Set the material Id to the correct material. */
			curMesh->Material = i;
			return;
		}
	}

	/* Throw if we cannot find the material. */
	LOG_THROW("Use of undefined material: '%s'!", line);
}

/* Handles the load material line. */
inline void HandleLoadMaterialLine(const char *line, const char *dir, ObjLoaderResult *result)
{
	/* Create buffer space for a maximum of 16 files. */
	constexpr size_t BUFFER_LEN = 16;
	char *buffer[BUFFER_LEN];
	for (size_t i = 0; i < BUFFER_LEN; i++) buffer[i] = malloca_s(char, FILENAME_MAX);

	/* Perform split and check for empty result. */
	size_t fileCnt = spltstr(line, ' ', buffer, 0);
	if (fileCnt < 1) LOG_WAR("No files defined in mtllib statement, use default material!");

	/* Parse material library. */
	for (size_t i = 0; i < fileCnt; i++)
	{
		LoadMaterialLibraryFromFile(dir, buffer[i], result);
		freea_s(buffer[i]);
	}
}

/* Handles the group name line. */
void HandleGroupNameLine(const char *line, ObjLoaderResult *result, ObjLoaderMesh *curMesh)
{
	/* Add old shape if needed. */
	PushShapeIfNeeded(result, curMesh, false);

	/* Make sure we skip leading spaces. */
	SkipUseless(&line);

	/* Set mesh name. */
	curMesh->Name = heapstr(line);
}

/* Handles the object name line. */
void HandleObjectNameLine(const char *line, ObjLoaderResult *result, ObjLoaderMesh *curMesh)
{
	/* Add old shape if needed. */
	PushShapeIfNeeded(result, curMesh, false);

	/* Make sure we skip leading spaces. */
	SkipUseless(&line);

	/* Set mesh name. */
	curMesh->Name = heapstr(line);
}

/* Handles the smoothing group line. */
void HandleSmoothingGroupLine(const char *line, uint64 *smoothingGroup)
{
	/* Skip leading spaces. */
	SkipUseless(&line);

	/* Skip empty lines. */
	if (IS_NEWLINE(line[0])) return;

	/* Set smoothing group to zero is 'off' is specified. */
	if (::strlen(line) >= 3)
	{
		if (line[0] == 'o' && line[1] == 'f' && line[2] == 'f') *smoothingGroup = 0;
	}
	else
	{
		/* Try parse the group. */
		int group = ParseInt(&line);
		if (group < 0)
		{
			/* Invalid value or parsing error. */
			LOG_WAR("Invalid smoothing group (%d) specified, defaulting to zero!", group);
			*smoothingGroup = 0;
		}
		else *smoothingGroup = static_cast<int64>(group);
	}
}

/* Handles the raw line within an obj file. */
void HandleObjLine(const char *line, const char *dir, ObjLoaderResult *result, ObjLoaderMesh *curMesh, uint64 *smoothingGroup)
{
	/* Skip leading spaces. */
	SkipUseless(&line);

	/* Skip empty lines and comments. */
	if (line[0] == '\0' || line[0] == '#') return;

	/* Check if line is vertex definition. */
	if (line[0] == 'v' && IS_SPACE(line[1]))
	{
		HandleVertexLine(line + 2, result);
		return;
	}

	/* Check if line is normal definition. */
	if (line[0] == 'v' && line[1] == 'n' && IS_SPACE(line[2]))
	{
		HandleNormalLine(line + 3, result);
		return;
	}

	/* Check if line is texture coordinate definition. */
	if (line[0] == 'v' && line[1] == 't' && IS_SPACE(line[2]))
	{
		HandleTexCoordLine(line + 3, result);
		return;
	}

	/* Check if line is face definition. */
	if (line[0] == 'f' && IS_SPACE(line[1]))
	{
		HandleFaceLine(line + 2, result, curMesh, *smoothingGroup);
		return;
	}

	/* Check if line is use material line. */
	if (!strncmp(line, "usemtl", 6) && IS_SPACE(line[6]))
	{
		HandleUseMaterialLine(line + 7, result, curMesh);
		return;
	}

	/* Check if line is load material line. */
	if (!strncmp(line, "mtllib", 6) && IS_SPACE(line[6]))
	{
		HandleLoadMaterialLine(line + 7, dir, result);
		return;
	}

	/* Check if line is group name line. */
	if (line[0] == 'g' && IS_SPACE(line[1]))
	{
		HandleGroupNameLine(line + 2, result, curMesh);
		return;
	}

	/* Check if line is object name. */
	if (line[0] == 'o' && IS_SPACE(line[1]))
	{
		HandleObjectNameLine(line + 2, result, curMesh);
		return;
	}

	/* Check if line is smoothing group. */
	if (line[0] == 's' && IS_SPACE(line[1]))
	{
		HandleSmoothingGroupLine(line + 2, smoothingGroup);
		return;
	}

	LOG_WAR("Unknown token on line: '%s'!", line);
}
#pragma endregion

#pragma region Mtl Line handling
/* Handles the new material line. */
inline void HandleNewMaterialLine(const char *line, ObjLoaderResult *result, ObjLoaderMaterial *curMaterial)
{
	/* Push old material if needed. */
	if (::strlen(curMaterial->Name) > 0)
	{
		result->Materials.push_back(*curMaterial);
		*curMaterial = ObjLoaderMaterial();
	}

	/* Set name. */
	curMaterial->Name = heapstr(line);
}

/* Handles the ambient light line. */
inline void HandleAmbientLightLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->Ambient = ParseColor3(&line);
}

/* Handles the diffuse light line. */
inline void HandleDiffuseLightLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->Diffuse = ParseColor3(&line);
}

/* Handles the specular light line. */
inline void HandleSpecularLightLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->Specular = ParseColor3(&line);
}

/* Handles the light transmittance line. */
inline void HandleTransmittanceLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->Transmittance = ParseColor3(&line);
}

/* Handles the index of refraction line. */
inline void HandleIoRLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->OpticalDensity = ParseFloat(&line);
}

/* Handles the shininess line. */
inline void HandleShininessLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->HighlightExponent = ParseFloat(&line);
}

/* Handles the illumination model line. */
inline void HandleIllumModelLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->IlluminationModel = ParseInt(&line);
}

/* Handles the dissolve line. */
inline void HandleDissolveLine(const char *line, ObjLoaderMaterial *curMaterial, bool *hasd, bool *hastr)
{
	float value = ParseFloat(&line);
	LOG_WAR_IF(*hastr && curMaterial->Dissolve != value, "Both 'd' and 'Tr' are defined withing material '%s', using value of 'd' for dissolve!", curMaterial->Name);

	*hasd = true;
	curMaterial->Dissolve = value;
}

/* Handles the inverse dissolve line. */
inline void HandleInverseDissolveLine(const char *line, ObjLoaderMaterial *curMaterial, bool *hasd, bool *hastr)
{
	float value = 1.0f - ParseFloat(&line);
	if (*hasd)
	{
		LOG_WAR_IF(curMaterial->Dissolve != value, "Both 'd' and 'Tr' are defined withing material '%s', using value of 'd' for dissolve!", curMaterial->Name);
	}
	else
	{
		*hastr = true;
		curMaterial->Dissolve = value;
	}
}

/* Handles the ambient texture line. */
inline void HandleTextureLine(const char *line, const char *dir, ObjLoaderTextureMap *curTexture, bool isBump)
{
	/* In case the texture already has some values. */
	bool checkOld = ::strlen(curTexture->Path) > 0;
	ObjLoaderTextureMap old(*curTexture);
	*curTexture = ObjLoaderTextureMap(isBump);

	/* Handle all options. */
	while (!IS_NEWLINE(*line))
	{
		/* Skip spaces. */
		SkipUseless(&line);

		/* Check if option is horizontal blending. */
		if (!strncmp(line, "-blendu", 7) && IS_SPACE(line[7]))
		{
			line += 8;
			curTexture->BlendH = ParseOnOff(&line);
			continue;
		}

		/* Check if option is vertical blending. */
		if (!strncmp(line, "-blendv", 7) && IS_SPACE(line[7]))
		{
			line += 8;
			curTexture->BlendV = ParseOnOff(&line);
			continue;
		}

		/* Check if option is texture coordinate clamping. */
		if (!strncmp(line, "-clamp", 6) && IS_SPACE(line[6]))
		{
			line += 7;
			curTexture->ClampedCoords = ParseOnOff(&line);
			continue;
		}

		/* Check if option is sharpness. */
		if (!strncmp(line, "-boost", 6) && IS_SPACE(line[6]))
		{
			line += 7;
			curTexture->Sharpness = ParseFloat(&line, 1.0f);
			continue;
		}

		/* Check if option is bump multiplier. */
		if (!strncmp(line, "-bm", 3) && IS_SPACE(line[3]))
		{
			line += 4;
			curTexture->BumpMod = ParseFloat(&line, 1.0f);
			continue;
		}

		/* Check if option is coordinate origin. */
		if (!strncmp(line, "-o", 2) && IS_SPACE(line[2]))
		{
			line += 3;
			curTexture->Origin = ParseFloat3(&line);
			continue;
		}

		/* Check if option is coordinate scale. */
		if (!strncmp(line, "-s", 2) && IS_SPACE(line[2]))
		{
			line += 3;
			curTexture->Scale = ParseFloat3(&line, 1.0f, 1.0f, 1.0f);
			continue;
		}

		/* Check if option is coordinate turbulence. */
		if (!strncmp(line, "-t", 2) && IS_SPACE(line[2]))
		{
			line += 3;
			curTexture->Turbulence = ParseFloat3(&line);
			continue;
		}

		/* Check if option is texture type. */
		if (!strncmp(line, "-type", 5) && IS_SPACE(5))
		{
			line += 6;
			curTexture->Type = ParseMapType(&line);
			continue;
		}

		/* Check if option is scalar or bump texture channel. */
		if (!strncmp(line, "-imfchan", 8) && IS_SPACE(line[8]))
		{
			line += 9;
			curTexture->ScalarOrBumpChannel = ParseChannel(&line);
			continue;
		}

		/* Check if option is brightness. */
		if (!strncmp(line, "-mm", 3) && IS_SPACE(line[3]))
		{
			line += 4;
			curTexture->Brightness = ParseFloat(&line);
			curTexture->Contrast = ParseFloat(&line, 1.0f);
			continue;
		}

		/* Assume texture path. */
		curTexture->Path = malloc_s(char, FILENAME_MAX);
		mrgstr(dir, line, const_cast<char*>(curTexture->Path));
		line += ::strlen(line);
	}

	/* Check for errors or warnings. */
	LOG_THROW_IF(::strlen(curTexture->Path) < 1, "Texture defines no path!");
	LOG_WAR_IF(checkOld && old != *curTexture, "Texture map is defined multiple times, replacing old texture map '%s'.", curTexture->Path);
}

/* Handles the raw line of a mtl file. */
void HandleMtlLine(const char *line, const char *dir, ObjLoaderResult *result, ObjLoaderMaterial *curMaterial, bool *hasd, bool *hastr)
{
	/* Skip leading spaces. */
	SkipUseless(&line);

	/* Skip empty lines and comments. */
	if (line[0] == '\0' || line[0] == '#') return;

	/* Check if line is new material line. */
	if (!strncmp(line, "newmtl", 6) && IS_SPACE(line[6]))
	{
		HandleNewMaterialLine(line + 7, result, curMaterial);
		*hasd = false;
		*hastr = false;
		return;
	}

	/* Check if line is ambient line. */
	if (line[0] == 'K' && line[1] == 'a' && IS_SPACE(line[2]))
	{
		HandleAmbientLightLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is diffuse line. */
	if (line[0] == 'K' && line[1] == 'd' && IS_SPACE(line[2]))
	{
		HandleDiffuseLightLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is specular line. */
	if (line[0] == 'K' && line[1] == 's' && IS_SPACE(line[2]))
	{
		HandleSpecularLightLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is transmittance line. */
	if ((line[0] == 'K' && line[1] == 't' && IS_SPACE(line[2])) ||
		(line[0] == 'T' && line[1] == 'f' && IS_SPACE(line[2])))
	{
		HandleTransmittanceLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is index of refraction. */
	if (line[0] == 'N' && line[1] == 'i' && IS_SPACE(line[2]))
	{
		HandleIoRLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is shininess. */
	if (line[0] == 'N' && line[1] == 's' && IS_SPACE(line[2]))
	{
		HandleShininessLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is illumination model. */
	if (!strncmp(line, "illum", 5) && IS_SPACE(line[5]))
	{
		HandleIllumModelLine(line + 5, curMaterial);
		return;
	}

	/* Check if line is dissolve. */
	if (line[0] == 'd' && IS_SPACE(line[1]))
	{
		HandleDissolveLine(line + 1, curMaterial, hasd, hastr);
		return;
	}

	/* Check if line is inverse dissolve. */
	if (line[0] == 'T' && line[1] == 'r' && IS_SPACE(line[2]))
	{
		HandleInverseDissolveLine(line + 2, curMaterial, hasd, hastr);
		return;
	}

	/* Check if line is ambient tex line. */
	if (!strncmp(line, "map_Ka", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, &curMaterial->AmbientMap, false);
		return;
	}

	/* Check if line is diffuse tex line. */
	if (!strncmp(line, "map_Kd", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, &curMaterial->DiffuseMap, false);
		return;
	}

	/* Check if line is specular tex line. */
	if (!strncmp(line, "map_Ks", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, &curMaterial->SpecularMap, false);
		return;
	}

	/* Check if line is specular highlight tex line. */
	if (!strncmp(line, "map_Ns", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, &curMaterial->HighlightMap, false);
		return;
	}

	/* Check if line is bump tex line. */
	if ((!strncmp(line, "map_Bump", 8) || !strncmp(line, "map_bump", 8)) && IS_SPACE(line[8]))
	{
		HandleTextureLine(line + 9, dir, &curMaterial->BumpMap, true);
		return;
	}

	/* Check if line is bump tex line. */
	if (!strncmp(line, "bump", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, &curMaterial->BumpMap, true);
		return;
	}

	/* Check if line is alpha tex line. */
	if (!strncmp(line, "map_d", 5) && IS_SPACE(line[5]))
	{
		HandleTextureLine(line + 6, dir, &curMaterial->AlphaMap, true);
		return;
	}

	/* Check if line is displacement tex line. */
	if (!strncmp(line, "disp", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, &curMaterial->DisplacementMap, true);
		return;
	}

	/* Check if line is reflection tex line. */
	if (!strncmp(line, "refl", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, &curMaterial->ReflectionMap, true);
		return;
	}

	LOG_WAR("Unknown token on line: '%s'!", line);
}
#pragma endregion

#pragma region Loading
void LoadMaterialLibraryFromFile(const char *dir, const char *name, ObjLoaderResult *result)
{
	/* Create reader to the file. */
	char *path = malloca_s(char, FILENAME_MAX);
	mrgstr(dir, name, path);
	FileReader reader(path);

	/* Define current material. */
	ObjLoaderMaterial material;
	bool hasd = false, hastr = false;

	/* Read untill the end of the file. */
	while (reader.Peek() != EOF)
	{
		const char *line = reader.ReadLine();

		/* Only handle line if it's not an empty line. */
		if (::strlen(line) > 0) HandleMtlLine(line, dir, result, &material, &hasd, &hastr);
		free_s(line);
	}

	/* Add last material to result if needed and release path. */
	if (::strlen(material.Name) > 0) result->Materials.push_back(material);
	freea_s(path);
}

const ObjLoaderResult * Plutonium::_CrtLoadObjMtl(const char * path, std::atomic<float>* progression, float progressionMod)
{
	/* Setup input and open obj file. */
	ObjLoaderResult *result = new ObjLoaderResult();
	FileReader reader(path);

	/* Defines current mesh and smoothing group. */
	ObjLoaderMesh shape;
	uint64 smoothingGroup;

	/* Read untill the end of the file. */
	float fileLength = recip(static_cast<float>(reader.GetSize()));
	while (reader.Peek() != EOF)
	{
		const char *line = reader.ReadLine();

		/* Only handle line if it's not an empty line. */
		if (strlen(line) > 0)
		{
			if (progression)
			{
				int64 pos = reader.GetPosition();
				progression->store(static_cast<float>(pos) * fileLength * progressionMod);
			}

			HandleObjLine(line, reader.GetFileDirectory(), result, &shape, &smoothingGroup);
		}
		free_s(line);
	}

	/* Add last shape to result if needed and return. */
	if (strlen(shape.Name) > 0) result->Shapes.push_back(shape);
	return result;
}
#pragma endregion