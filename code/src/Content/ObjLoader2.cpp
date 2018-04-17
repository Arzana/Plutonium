#include "Content\ObjLoader2.h"
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

void LoadMaterialLibraryFromFile(const char*, const char*, ObjLoaderResult2*);

#pragma region Constructors
Plutonium::ObjLoaderVertex::ObjLoaderVertex(void)
	: Vertex(-1), Normal(-1), TexCoord(-1)
{}

Plutonium::ObjLoaderMesh::ObjLoaderMesh(void)
	: Name(""), Indices(), VerticesPerFace(), Material(-1), SmoothingGroups()
{}

Plutonium::ObjLoaderTextureMap::ObjLoaderTextureMap(bool isBump)
	: Path(""), Type(ObjLoaderMapType::None),
	Sharpness(1.0f), Brightness(0.0f), Contrast(1.0f),
	Origin(), Scale(1.0f), Turbulence(), ClampedCoords(false),
	BlendH(true), BlendV(true),
	ScalarOrBumpChannel(isBump ? ObjLoaderChannel::Luminance : ObjLoaderChannel::Matte), BumpMod(1.0f)
{}

Plutonium::ObjLoaderMaterial::ObjLoaderMaterial(void)
	: Name(""),
	Ambient(Color::Black), Diffuse(Color::Black), Specular(Color::Black),
	Transmittance(Color::Black), HighlightExponent(1.0f), OpticalDensity(1.0f), Dissolve(1.0f),
	AmbientMap(false), DiffuseMap(false), SpecularMap(false), HighlightMap(false),
	BumpMap(true), DisplacementMap(false), AlphaMap(false), ReflectionMap(false)
{}

Plutonium::ObjLoaderResult2::ObjLoaderResult2(void)
	: Vertices(), Normals(), TexCoords(), Shapes(), Materials()
{}
#pragma endregion

#pragma region Checks / moves
/* Gets whether a specified character is a space. */
#define IS_SPACE(x)		((x) == ' ' || (x) == '\t')

/* Gets whether a specified character is a newline. */
#define IS_NEWLINE(x)	((x) == '\n' || (x) == '\r' || (x) == '\0')

/* Skips leading spaces and tabs. */
void SkipUseless(const char **line)
{
	while (IS_SPACE(*line[0])) ++(*line);
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
	(*line) = end;
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
	(*line) = end;
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
	(*line) = end;
	return result;
}

float ParseFloat(const char **line, float def = 0.0f)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = (*line) + strcspn((*line), " \t\r");

	/* Parse value. */
	float result = def;
	tryParseFloat(*line, end, &result);

	/* Increase line position and return result. */
	(*line) = end;
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
#pragma endregion

#pragma region Obj Line handling
/* Handles the vertex line. */
inline void HandleVertexLine(const char *line, ObjLoaderResult2 *result)
{
	result->Vertices.push_back(ParseFloat3(&line));
}

/* Handles the normal line. */
inline void HandleNormalLine(const char *line, ObjLoaderResult2 *result)
{
	result->Normals.push_back(ParseFloat3(&line));
}

/* Handles the texture coordinate line. */
inline void HandleTexCoordLine(const char *line, ObjLoaderResult2 *result)
{
	result->TexCoords.push_back(ParseFloat2(&line));
}

/* Handles the face line. */
inline void HandleFaceLine(const char *line, ObjLoaderResult2 *result, ObjLoaderMesh *curMesh, uint64 smoothingGroup)
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
	while (!IS_NEWLINE(line[0]))
	{
		/* Try parse the value and throw is failed (file corrupt). */
		ObjLoaderVertex vi;
		if (!TryParseTriple(&line, vSize, vnSize, vtSize, &vi)) LOG_THROW("Failed to parse face!\nLINE:		%s", line);

		/* Add face to mesh. */
		curMesh->Indices.push_back(vi);
		line += strspn(line, " \t\r");
	}

	// Add vertices per face?
}

/* Handles the use material line. */
inline void HandleUseMaterialLine(const char *line, ObjLoaderResult2 *result, ObjLoaderMesh *curMesh)
{
	/* Search for the material in the defined list. */
	for (size_t i = 0; i < result->Materials.size(); i++)
	{
		if (!strcmp(result->Materials.at(i).Name, line))
		{
			LOG_WAR_IF(curMesh->Material != -1, "Redefining material for shape %s (%s -> %s)!", curMesh->Name, result->Materials.at(curMesh->Material).Name, line);
			curMesh->Material = i;
			return;
		}
	}

	/* Throw if we cannot find the material. */
	LOG_THROW("Use of undefined material: '%s'!", line);
}

/* Handles the load material line. */
inline void HandleLoadMaterialLine(const char *line, const char *dir, ObjLoaderResult2 *result)
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

/* Handles the raw line within an obj file. */
void HandleObjLine(const char *line, const char *dir, ObjLoaderResult2 *result, ObjLoaderMesh *curMesh, uint64 *smoothingGroup)
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

	// Add group names, obj names, etc.


	LOG_WAR("Unknown token on line: '%s'!", line);
}
#pragma endregion

#pragma region Mtl Line handling
/* Handles the new material line. */
inline void HandleNewMaterialLine(const char *line, ObjLoaderResult2 *result, ObjLoaderMaterial *curMaterial)
{
	/* Push old material if needed. */
	if (strlen(curMaterial->Name) > 0)
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

/* Handles the dissolve line. */
inline void HandleDissolveLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->Dissolve = ParseFloat(&line);
}

/* Handles the inverse dissolve line. */
inline void HandleInverseDissolveLine(const char *line, ObjLoaderMaterial *curMaterial)
{
	curMaterial->Dissolve = 1.0f - ParseFloat(&line);
}

/* Handles the ambient texture line. */
inline void HandleTextureLine(const char *line, ObjLoaderTextureMap *curTexture, bool isBump)
{
	/* Reset texture map values. */
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
		curTexture->Path = heapstr(line);
		line += strlen(curTexture->Path);
	}

	LOG_THROW_IF(strlen(curTexture->Path) < 1, "Texture defines no path!");
}

/* Handles the raw line of a mtl file. */
void HandleMtlLine(const char *line, ObjLoaderResult2 *result, ObjLoaderMaterial *curMaterial, bool *hasd, bool *hastr)
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

	/* Check if line is dissolve. */
	if (line[0] == 'd' && IS_SPACE(line[1]))
	{
		LOG_WAR_IF(*hastr, "Both 'd' and 'Tr' are defined withing material '%s', using value of 'd' for dissolve!", curMaterial->Name);
		*hasd = true;
		HandleDissolveLine(line + 1, curMaterial);
		return;
	}

	/* Check if line is inverse dissolve. */
	if (line[0] == 'T' && line[1] == 'r' && IS_SPACE(line[2])) 
	{
		if (*hasd) LOG_WAR("Both 'd' and 'Tr' are defined withing material '%s', using value of 'd' for dissolve!", curMaterial->Name);
		else
		{
			*hastr = true;
			HandleInverseDissolveLine(line + 2, curMaterial);
		}

		return;
	}

	/* Check if line is ambient tex line. */
	if (!strncmp(line, "map_Ka", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, &curMaterial->AmbientMap, false);
		return;
	}

	/* Check if line is diffuse tex line. */
	if (!strncmp(line, "map_Kd", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, &curMaterial->DiffuseMap, false);
		return;
	}

	/* Check if line is specular tex line. */
	if (!strncmp(line, "map_Ks", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, &curMaterial->SpecularMap, false);
		return;
	}

	/* Check if line is specular highlight tex line. */
	if (!strncmp(line, "map_Ns", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, &curMaterial->HighlightMap, false);
		return;
	}

	/* Check if line is bump tex line. */
	if ((!strncmp(line, "map_Bump", 8) || !strncmp(line, "map_bump", 8)) && IS_SPACE(line[8]))
	{
		HandleTextureLine(line + 9, &curMaterial->BumpMap, true);
		return;
	}

	/* Check if line is bump tex line. */
	if (!strncmp(line, "bump", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, &curMaterial->BumpMap, true);
		return;
	}

	/* Check if line is alpha tex line. */
	if (!strncmp(line, "map_d", 5) && IS_SPACE(line[5]))
	{
		HandleTextureLine(line + 6, &curMaterial->AlphaMap, true);
		return;
	}

	/* Check if line is displacement tex line. */
	if (!strncmp(line, "disp", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, &curMaterial->DisplacementMap, true);
		return;
	}

	/* Check if line is reflection tex line. */
	if (!strncmp(line, "refl", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, &curMaterial->ReflectionMap, true);
		return;
	}

	LOG_WAR("Unknown token on line: '%s'!", line);
}
#pragma endregion

#pragma region Loading
void LoadMaterialLibraryFromFile(const char *dir, const char *name, ObjLoaderResult2 *result)
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
		if (strlen(line) > 0) HandleMtlLine(line, result, &material, &hasd, &hastr);
		free_s(line);
	}

	/* Add last material to result if needed and release path. */
	if (strlen(material.Name) > 0) result->Materials.push_back(material);
	freea_s(path);
}

const ObjLoaderResult2 * Plutonium::_CrtLoadObjMtl2(const char * path)
{
	/* Setup input and open obj file. */
	ObjLoaderResult2 *result = new ObjLoaderResult2();
	FileReader reader(path);

	/* Defines current mesh and smoothing group. */
	ObjLoaderMesh shape;
	uint64 smoothingGroup;

	/* Read untill the end of the file. */
	while (reader.Peek() != EOF)
	{
		const char *line = reader.ReadLine();

		/* Only handle line if it's not an empty line. */
		if (strlen(line) > 0) HandleObjLine(line, reader.GetFileDirectory(), result, &shape, &smoothingGroup);
		free_s(line);
	}

	/* Add last shape to result if needed and return. */
	if (strlen(shape.Name) > 0) result->Shapes.push_back(shape);
	return result;
}
#pragma endregion