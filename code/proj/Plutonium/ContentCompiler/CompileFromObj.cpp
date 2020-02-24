#include "CompileFromObj.h"
#include "Streams/FileReader.h"
#include "Core/EnumUtils.h"
#include "Core/Math/Triangulation.h"

/*
Based on:
Syoyo Fujita.
tinyobjloader (2012)
https://github.com/syoyo/tinyobjloader
*/

using namespace Pu;

void LoadMaterialLibraryFromFile(const string&, const char*, ObjLoaderResult&);

#pragma region Constructors
ObjLoaderVertex::ObjLoaderVertex(void)
	: Vertex(-1), Normal(-1), TexCoord(-1)
{}

ObjLoaderMesh::ObjLoaderMesh(void)
	: Name(""), Indices(), Material(-1), SmoothingGroups()
{}

ObjLoaderTextureMap::ObjLoaderTextureMap(bool isBump)
	: Path(""), Type(ObjLoaderMapType::None),
	Sharpness(1.0f), Brightness(0.0f), Contrast(1.0f),
	Origin(), Scale(1.0f), Turbulence(), ClampedCoords(false),
	BlendH(true), BlendV(true),
	ScalarOrBumpChannel(isBump ? ObjLoaderChannel::Luminance : ObjLoaderChannel::Matte), BumpMod(1.0f)
{}

bool ObjLoaderTextureMap::operator!=(const ObjLoaderTextureMap & other) const
{
	if (Path == other.Path) return false;
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

ObjLoaderMaterial::ObjLoaderMaterial(void)
	: Name(""), IlluminationModel(0),
	Ambient(Color::Black()), Diffuse(Color::Black()), Specular(Color::Black()), Emissive(Color::Black()),
	Transmittance(Color::White()), HighlightExponent(1.0f), OpticalDensity(1.0f), Dissolve(1.0f),
	Glossiness(1.0f), Metallic(0.0f),
	AmbientMap(false), AlbedoMap(false), SpecularMap(false), EmissiveMap(false),
	HighlightMap(false), BumpMap(true), DisplacementMap(false), AlphaMap(false), ReflectionMap(false),
	GlossinessMap(false), MetallicMap(false)
{}

ObjLoaderResult::ObjLoaderResult(void)
	: Vertices(), Normals(), TexCoords(), Shapes(), Materials()
{}

int32 ObjLoaderResult::GetDefaultMaterial(void) const
{
	int32 i = 0;
	for (const ObjLoaderMaterial &cur : Materials)
	{
		if (cur.Name == "Default") return i;
		else i++;
	}

	return -1;
}
#pragma endregion

#pragma region Checks / moves
/* Gets whether a specified character is a space. */
#define IS_SPACE(x)		((x) == ' ' || (x) == '\t')

/* Gets whether a specified character is a newline. */
#define IS_NEWLINE(x)	((x) == '\n' || (x) == '\r' || (x) == '\0')

/* Skips leading spaces and tabs. */
inline void SkipUseless(const char *&line)
{
	while (IS_SPACE(line[0])) ++line;
}

/* Pushes the old mesh to the result and resets the current mesh, return whether the mesh was pushed. */
bool PushShapeIfNeeded(ObjLoaderResult &result, ObjLoaderMesh &curMesh, bool keepName)
{
	const string name = curMesh.Name;

	/* Check if current mesh has data. */
	if (!(curMesh.Name.empty() || curMesh.Indices.empty()))
	{
		if (curMesh.Material == -1) Log::Verbose("Pushing shape '%s' with no material defined!", curMesh.Name.c_str());

		/* Push old mesh. */
		result.Shapes.emplace_back(curMesh);
		curMesh = ObjLoaderMesh();

		/* Keep the old name if requested. */
		if (keepName) curMesh.Name = name;
		return true;
	}

	return false;
}
#pragma endregion

#pragma region Parsing
bool ParseOnOff(const char *&line, bool def = true)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = line + strcspn(line, " \t\r");

	/* Parse value. */
	bool result = def;
	if (!strncmp(line, "on", 2)) result = true;
	else if (!strncmp(line, "off", 3)) result = false;

	/* Increase line position and return result. */
	line = end;
	return result;
}

ObjLoaderMapType ParseMapType(const char *&line, ObjLoaderMapType def = ObjLoaderMapType::None)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = line + strcspn(line, " \t");

	/* Parse value. */
	ObjLoaderMapType result = def;
	if (!strncmp(line, "sphere", 6)) result = ObjLoaderMapType::Sphere;
	else if (!strncmp(line, "cube_left", 9)) result = ObjLoaderMapType::CubeLeft;
	else if (!strncmp(line, "cube_right", 10)) result = ObjLoaderMapType::CubeRight;
	else if (!strncmp(line, "cube_front", 10)) result = ObjLoaderMapType::CubeFront;
	else if (!strncmp(line, "cube_back", 9)) result = ObjLoaderMapType::CubeBack;
	else if (!strncmp(line, "cube_top", 8)) result = ObjLoaderMapType::CubeTop;
	else if (!strncmp(line, "cube_bottom", 11)) result = ObjLoaderMapType::CubeBottom;

	/* Increase line position and return result. */
	line = end;
	return result;
}

ObjLoaderChannel ParseChannel(const char *&line, ObjLoaderChannel def = ObjLoaderChannel::None)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = line + strcspn(line, " \t");

	/* Parse value. */
	ObjLoaderChannel result = def;
	if ((end - line) == 1) result = _CrtInt2Enum<ObjLoaderChannel>(*line);

	/* Increase line position and return result. */
	line = end;
	return result;
}

int ParseInt(const char *&line)
{
	/* get begin and final read point. */
	SkipUseless(line);
	const char *end = line + strcspn(line, " \t\r");

	/* Parse value. */
	int result = atoi(line);

	/* Increase line position and return result. */
	line = end;
	return result;
}

float ParseFloat(const char *&line, float def = 0.0f)
{
	/* Get begin and final read point. */
	SkipUseless(line);
	const char *end = line + strcspn(line, " \t\r");

	/* Parse value. */
	float result = std::strtof(line, nullptr);

	/* Increase line position and return result. */
	line = end;
	return result;
}

Vector2 ParseFloat2(const char *&line, float defX = 0.0f, float defY = 0.0f)
{
	float x = ParseFloat(line, defX);
	float y = ParseFloat(line, defY);
	return Vector2(x, y);
}

Vector3 ParseFloat3(const char *&line, float defX = 0.0f, float defY = 0.0f, float defZ = 0.0f)
{
	float x = ParseFloat(line, defX);
	float y = ParseFloat(line, defY);
	float z = ParseFloat(line, defZ);
	return Vector3(x, y, z);
}

Vector4 ParseFloat4(const char *&line, float defX = 0.0f, float defY = 0.0f, float defZ = 0.0f, float defW = 0.0f)
{
	float x = ParseFloat(line, defX);
	float y = ParseFloat(line, defY);
	float z = ParseFloat(line, defZ);
	float w = ParseFloat(line, defW);
	return Vector4(x, y, z, w);
}

Color ParseColor3(const char *&line, float defR = 0.0f, float defG = 0.0f, float defB = 0.0f)
{
	Vector3 v = ParseFloat3(line, defR, defG, defB);
	return Color(v.X, v.Y, v.Z);
}

bool TryFixIndex(int64 idx, int64 n, int64 &result)
{
	/* If input is positive, convert from normal to zero based. */
	if (idx > 0)
	{
		result = idx - 1;
		return true;
	}

	/* if input is negative, convert from normal to zero based and invert direction. */
	if (idx < 0)
	{
		result = n + idx;
		return true;
	}

	/* .obj doesn't allow for zero index values. */
	return false;
}

bool TryParseTriple(const char *&line, size_t vSize, size_t vnSize, size_t vtSize, ObjLoaderVertex &result)
{
	/* Parse vertex index and move read position. */
	if (!TryFixIndex(atoi(line), vSize, result.Vertex)) return false;
	line += strcspn(line, "/ \t\r");

	/* Check if end is reached. */
	if (line[0] != '/') return true;
	++line;

	if (line[0] == '/')
	{
		/* Parse normal index and move read position. */
		++line;
		if (!TryFixIndex(atoi(line), vnSize, result.Normal)) return false;
		line += strcspn(line, "/ \t\r");
		return true;
	}

	/* Parse texture coordinate index and move read position. */
	if (!TryFixIndex(atoi(line), vtSize, result.TexCoord)) return false;
	line += strcspn(line, "/ \t\r");

	/* Check if end is reached. */
	if (line[0] != '/') return true;
	++line;

	/* Parse normal index and move read position. */
	if (!TryFixIndex(atoi(line), vnSize, result.Normal)) return false;
	line += strcspn(line, "/ \t\r");

	return true;
}

void TriangulatePolygon(const ObjLoaderResult &result, std::vector<ObjLoaderVertex> &vertices)
{
	/* Used to store the vertices whilst they are being triangulated. */
	struct TmpVrtx
	{
		ObjLoaderVertex Vertex;
		Vector3 Vector;

		TmpVrtx(void)
		{}

		TmpVrtx(const ObjLoaderResult &result, ObjLoaderVertex vertex)
			: Vertex(vertex), Vector(result.Vertices[vertex.Vertex])
		{}
	};

	/* Check for invalid inputs. */
	for (const ObjLoaderVertex &vrtx : vertices)
	{
		if (vrtx.Vertex == -1 || static_cast<size_t>(vrtx.Vertex) >= result.Vertices.size())
		{
			Log::Fatal("Input vertex index (%d) is not defined!", vrtx.Vertex);
		}
	}

	/* Populate temporary buffer needed for triangulation function. */
	size_t size = (vertices.size() - 2) * 3;
	TmpVrtx *buffer = reinterpret_cast<TmpVrtx*>(malloc(sizeof(TmpVrtx) * size));
	for (size_t i = 0; i < vertices.size(); i++) buffer[i] = TmpVrtx(result, vertices[i]);

	/* Use specialized faster function for quads. */
	if (vertices.size() == 4) triangulateQuad(buffer, sizeof(TmpVrtx), offsetof(TmpVrtx, Vector));
	else size = triangulateConvex(buffer, sizeof(TmpVrtx), vertices.size());

	/* Copy the triangulated vertexes back to the vertices list. */
	vertices.clear();
	vertices.reserve(size);
	for (size_t i = 0; i < size; i++) vertices.emplace_back(buffer[i].Vertex);
	free(buffer);
}
#pragma endregion

#pragma region Obj Line handling
/* Handles the vertex line. */
inline void HandleVertexLine(const char *line, ObjLoaderResult &result)
{
	result.Vertices.emplace_back(ParseFloat3(line));
}

/* Handles the normal line. */
inline void HandleNormalLine(const char *line, ObjLoaderResult &result)
{
	result.Normals.emplace_back(ParseFloat3(line));
}

/* Handles the texture coordinate line. */
inline void HandleTexCoordLine(const char *line, ObjLoaderResult &result)
{
	const Vector2 uv = ParseFloat2(line); // The coordinates in OBJ are made to work with OpenGL so we need to invert them on the Y axis.
	result.TexCoords.emplace_back(Vector2(uv.X, 1.0f - uv.Y));
}

/* Handles the face line. */
inline void HandleFaceLine(const char *line, ObjLoaderResult &result, ObjLoaderMesh &curMesh, uint64 smoothingGroup)
{
	/* Skip leading spaces. */
	SkipUseless(line);

	/* Add current face's smoothing group. */
	curMesh.SmoothingGroups.emplace_back(smoothingGroup);

	/* Get required buffer sizes. */
	const size_t vSize = result.Vertices.size() / 3;
	const size_t vnSize = result.Normals.size() / 3;
	const size_t vtSize = result.TexCoords.size() / 2;

	/* Read untill no more faces are found. */
	std::vector<ObjLoaderVertex> face;
	while (!IS_NEWLINE(line[0]))
	{
		/* Try parse the value and throw is failed (file corrupt). */
		ObjLoaderVertex vi;
		if (!TryParseTriple(line, vSize, vnSize, vtSize, vi)) Log::Fatal("Failed to parse face ('%s')!", line);

		/* Add face to mesh. */
		face.emplace_back(vi);
		line += strspn(line, " \t\r");
	}

	/* Check if it's an invalid face. */
	if (face.size() < 3)
	{
		Log::Warning("Cannot convert face with %zu vertices into triangles, skipping face!", face.size());
		return;
	}

	/* Triangulate it and make sure to add the smoothing group for the new triangle if it's a polygon. */
	if (face.size() > 3)
	{
		TriangulatePolygon(result, face);

		/* Skip the face if the triangulation failed. */
		if (face.size() < 1)
		{
			Log::Warning("Failed to traingulate face, skipping face!");
			return;
		}

		curMesh.SmoothingGroups.emplace_back(smoothingGroup);
	}

	/* Push vertex count to mesh. */
	curMesh.Indices.insert(curMesh.Indices.end(), face.begin(), face.end());
}

/* Handles the use material line. */
inline void HandleUseMaterialLine(const char *line, ObjLoaderResult &result, ObjLoaderMesh &curMesh)
{
	/* Search for the material in the defined list. */
	for (size_t i = 0; i < result.Materials.size(); i++)
	{
		if (result.Materials[i].Name == line)
		{
			/* Check if the material is already defined. */
			if (curMesh.Material != -1)
			{
				/* Try to push the shape, we we succeed log a warning that the shape was split, otherwise it's proper use. */
				const size_t oldMtlIdx = curMesh.Material;
				if (PushShapeIfNeeded(result, curMesh, true))
				{
					Log::Verbose("Redefined material for shape %s (%s -> %s), creating new shape!", curMesh.Name.c_str(), result.Materials[oldMtlIdx].Name.c_str(), line);
				}
			}

			/* Set the material Id to the correct material. */
			curMesh.Material = i;
			return;
		}
	}

	/* Throw if we cannot find the material. */
	Log::Fatal("Use of undefined material: '%s'!", line);
}

/* Handles the load material line. */
inline void HandleLoadMaterialLine(const string &line, const string &dir, ObjLoaderResult &result, vector<string> &loadedMaterialLibraries)
{
	/* Get the amount of libraries defined in the line. */
	const size_t size = line.count(' ') + 1;
	if (size > 0)
	{
		/* Split line into material definitions. */
		vector<string> buffer = line.split(' ');

		/* Parse material library. */
		for (string &cur : buffer)
		{
			/* Only load the material library if it is not yet loaded. */
			bool alreadyLoaded = false;
			for (const string &check : loadedMaterialLibraries)
			{
				if (cur == check)
				{
					alreadyLoaded = true;
					break;
				}
			}

			if (!alreadyLoaded)
			{
				loadedMaterialLibraries.emplace_back(cur);
				LoadMaterialLibraryFromFile(dir, cur.c_str(), result);
			}
		}
	}
	else Log::Warning("No files defined in mtllib statement, use default material!");
}

/* Handles the group name line. */
void HandleGroupNameLine(const char *line, ObjLoaderResult &result, ObjLoaderMesh &curMesh)
{
	/* Add old shape if needed. */
	PushShapeIfNeeded(result, curMesh, false);

	/* Make sure we skip leading spaces. */
	SkipUseless(line);

	/* Set mesh name. */
	curMesh.Name = line;
}

/* Handles the object name line. */
void HandleObjectNameLine(const char *line, ObjLoaderResult &result, ObjLoaderMesh &curMesh)
{
	/* Add old shape if needed. */
	PushShapeIfNeeded(result, curMesh, false);

	/* Make sure we skip leading spaces. */
	SkipUseless(line);

	/* Set mesh name. */
	curMesh.Name = line;
}

/* Handles the smoothing group line. */
void HandleSmoothingGroupLine(const char *line, uint64 &smoothingGroup)
{
	/* Skip leading spaces. */
	SkipUseless(line);

	/* Skip empty lines. */
	if (IS_NEWLINE(line[0])) return;

	/* Set smoothing group to zero is 'off' is specified. */
	if (::strlen(line) >= 3)
	{
		if (line[0] == 'o' && line[1] == 'f' && line[2] == 'f') smoothingGroup = 0;
	}
	else
	{
		/* Try parse the group. */
		int group = ParseInt(line);
		if (group < 0)
		{
			/* Invalid value or parsing error. */
			Log::Warning("Invalid smoothing group (%d) specified, defaulting to zero!", group);
			smoothingGroup = 0;
		}
		else smoothingGroup = static_cast<int64>(group);
	}
}

/* Handles the raw line within an obj file. */
void HandleObjLine(const char *line, const string &dir, ObjLoaderResult &result, ObjLoaderMesh &curMesh, uint64 &smoothingGroup, vector<string> &loadedMaterialLibraries)
{
	/* Skip leading spaces. */
	SkipUseless(line);

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
		HandleFaceLine(line + 2, result, curMesh, smoothingGroup);
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
		HandleLoadMaterialLine(line + 7, dir, result, loadedMaterialLibraries);
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

	Log::Warning("Unknown token on line: '%s'!", line);
}
#pragma endregion

#pragma region Mtl Line handling
/* Handles the new material line. */
inline void HandleNewMaterialLine(const char *line, ObjLoaderResult &result, ObjLoaderMaterial &curMaterial)
{
	/* Push old material if needed. */
	if (!curMaterial.Name.empty())
	{
		result.Materials.emplace_back(curMaterial);
		curMaterial = ObjLoaderMaterial();
	}

	/* Set name. */
	curMaterial.Name = line;
}

/* Handles the ambient light line. */
inline void HandleAmbientLightLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Ambient = ParseColor3(line);
}

/* Handles the diffuse light line. */
inline void HandleDiffuseLightLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Diffuse = ParseColor3(line);
}

/* Handles the specular light line. */
inline void HandleSpecularLightLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Specular = ParseColor3(line);
}

/* Handles the light transmittance line. */
inline void HandleTransmittanceLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Transmittance = ParseColor3(line);
}

/* Handles the emissive light line. */
inline void HandleEmissiveLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Emissive = ParseColor3(line);
}

/* Handles the glossiness factor line. */
inline void HandleGlossinessLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Glossiness = ParseFloat(line);
}

/* Handles the metallic factor line. */
inline void HandleMetallicLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.Metallic = ParseFloat(line);
}

/* Handles the index of refraction line. */
inline void HandleIoRLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.OpticalDensity = ParseFloat(line);
}

/* Handles the shininess line. */
inline void HandleShininessLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.HighlightExponent = ParseFloat(line);
}

/* Handles the illumination model line. */
inline void HandleIllumModelLine(const char *line, ObjLoaderMaterial &curMaterial)
{
	curMaterial.IlluminationModel = ParseInt(line);
}

/* Handles the dissolve line. */
inline void HandleDissolveLine(const char *line, ObjLoaderMaterial &curMaterial, bool &hasd, bool &hastr)
{
	const float value = ParseFloat(line);
	if (hastr && curMaterial.Dissolve != value) Log::Verbose("Both 'd' and 'Tr' are defined within material '%s' using value of 'd' for dissolve!", curMaterial.Name.c_str());

	hasd = true;
	curMaterial.Dissolve = value;
}

/* Handles the inverse dissolve line. */
inline void HandleInverseDissolveLine(const char *line, ObjLoaderMaterial &curMaterial, bool &hasd, bool &hastr)
{
	const float value = 1.0f - ParseFloat(line);
	if (hasd)
	{
		if(curMaterial.Dissolve != value) Log::Verbose("Both 'd' and 'Tr' are defined within material '%s', using value of 'd' for dissolve!", curMaterial.Name.c_str());
	}
	else
	{
		hastr = true;
		curMaterial.Dissolve = value;
	}
}

/* Handles the ambient texture line. */
inline void HandleTextureLine(const char *line, const string &dir, ObjLoaderTextureMap &curTexture, bool isBump)
{
	/* In case the texture already has some values. */
	bool checkOld = !curTexture.Path.empty();
	ObjLoaderTextureMap old(curTexture);
	curTexture = ObjLoaderTextureMap(isBump);

	/* Handle all options. */
	while (!IS_NEWLINE(*line))
	{
		/* Skip spaces. */
		SkipUseless(line);

		/* Check if option is horizontal blending. */
		if (!strncmp(line, "-blendu", 7) && IS_SPACE(line[7]))
		{
			line += 8;
			curTexture.BlendH = ParseOnOff(line);
			continue;
		}

		/* Check if option is vertical blending. */
		if (!strncmp(line, "-blendv", 7) && IS_SPACE(line[7]))
		{
			line += 8;
			curTexture.BlendV = ParseOnOff(line);
			continue;
		}

		/* Check if option is texture coordinate clamping. */
		if (!strncmp(line, "-clamp", 6) && IS_SPACE(line[6]))
		{
			line += 7;
			curTexture.ClampedCoords = ParseOnOff(line);
			continue;
		}

		/* Check if option is sharpness. */
		if (!strncmp(line, "-boost", 6) && IS_SPACE(line[6]))
		{
			line += 7;
			curTexture.Sharpness = ParseFloat(line, 1.0f);
			continue;
		}

		/* Check if option is bump multiplier. */
		if (!strncmp(line, "-bm", 3) && IS_SPACE(line[3]))
		{
			line += 4;
			curTexture.BumpMod = ParseFloat(line, 1.0f);
			continue;
		}

		/* Check if option is coordinate origin. */
		if (!strncmp(line, "-o", 2) && IS_SPACE(line[2]))
		{
			line += 3;
			curTexture.Origin = ParseFloat3(line);
			continue;
		}

		/* Check if option is coordinate scale. */
		if (!strncmp(line, "-s", 2) && IS_SPACE(line[2]))
		{
			line += 3;
			curTexture.Scale = ParseFloat3(line, 1.0f, 1.0f, 1.0f);
			continue;
		}

		/* Check if option is coordinate turbulence. */
		if (!strncmp(line, "-t", 2) && IS_SPACE(line[2]))
		{
			line += 3;
			curTexture.Turbulence = ParseFloat3(line);
			continue;
		}

		/* Check if option is texture type. */
		if (!strncmp(line, "-type", 5) && IS_SPACE(5))
		{
			line += 6;
			curTexture.Type = ParseMapType(line);
			continue;
		}

		/* Check if option is scalar or bump texture channel. */
		if (!strncmp(line, "-imfchan", 8) && IS_SPACE(line[8]))
		{
			line += 9;
			curTexture.ScalarOrBumpChannel = ParseChannel(line);
			continue;
		}

		/* Check if option is brightness. */
		if (!strncmp(line, "-mm", 3) && IS_SPACE(line[3]))
		{
			line += 4;
			curTexture.Brightness = ParseFloat(line);
			curTexture.Contrast = ParseFloat(line, 1.0f);
			continue;
		}

		/* Assume texture path. */
		curTexture.Path = dir + line;
		line += ::strlen(line);
	}

	/* Check for errors or warnings. */
	if (curTexture.Path.empty()) Log::Fatal("Texture defines no path!");
	if (checkOld && old != curTexture) Log::Warning("Texture map is defined multiple times, replacing old texture map '%s'.", curTexture.Path.c_str());
}

/* Handles the raw line of a mtl file. */
void HandleMtlLine(const char *line, const string &dir, ObjLoaderResult &result, ObjLoaderMaterial &curMaterial, bool &hasd, bool &hastr)
{
	/* Skip leading spaces. */
	SkipUseless(line);

	/* Skip empty lines and comments. */
	if (line[0] == '\0' || line[0] == '#') return;

	/* Check if line is new material line. */
	if (!strncmp(line, "newmtl", 6) && IS_SPACE(line[6]))
	{
		HandleNewMaterialLine(line + 7, result, curMaterial);
		hasd = false;
		hastr = false;
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

	/* Check if line is emissive line. */
	if (line[0] == 'K' && line[1] == 'e' && IS_SPACE(line[2]))
	{
		HandleEmissiveLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is glossiness line. */
	if (line[0] == 'P' && line[1] == 'r' && IS_SPACE(line[2]))
	{
		HandleGlossinessLine(line + 2, curMaterial);
		return;
	}

	/* Check if line is metallic line. */
	if (line[0] == 'P' && line[1] == 'm' && IS_SPACE(line[2]))
	{
		HandleMetallicLine(line + 2, curMaterial);
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
		HandleTextureLine(line + 7, dir, curMaterial.AmbientMap, false);
		return;
	}

	/* Check if line is diffuse tex line. */
	if (!strncmp(line, "map_Kd", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, curMaterial.AlbedoMap, false);
		return;
	}

	/* Check if line is specular tex line. */
	if (!strncmp(line, "map_Ks", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, curMaterial.SpecularMap, false);
		return;
	}

	/* Check if line is specular highlight tex line. */
	if (!strncmp(line, "map_Ns", 6) && IS_SPACE(line[6]))
	{
		HandleTextureLine(line + 7, dir, curMaterial.HighlightMap, false);
		return;
	}

	/* Check if line is bump tex line. */
	if ((!strncmp(line, "map_Bump", 8) || !strncmp(line, "map_bump", 8)) && IS_SPACE(line[8]))
	{
		HandleTextureLine(line + 9, dir, curMaterial.BumpMap, true);
		return;
	}

	/* Check if line is bump tex line. */
	if (!strncmp(line, "bump", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, curMaterial.BumpMap, true);
		return;
	}

	/* Check if line is alpha tex line. */
	if (!strncmp(line, "map_d", 5) && IS_SPACE(line[5]))
	{
		HandleTextureLine(line + 6, dir, curMaterial.AlphaMap, true);
		return;
	}

	/* Check if line is displacement tex line. */
	if (!strncmp(line, "disp", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, curMaterial.DisplacementMap, true);
		return;
	}

	/* Check if line is reflection tex line. */
	if (!strncmp(line, "refl", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, curMaterial.ReflectionMap, true);
		return;
	}

	/* Check if line is glossiness tex line. */
	if (!strncmp(line, "map_Pr", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, curMaterial.GlossinessMap, false);
		return;
	}

	/* Check if line is metallic tex line. */
	if (!strncmp(line, "map_Pm", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, curMaterial.MetallicMap, false);
		return;
	}

	/* Check if line is emissive tex line. */
	if (!strncmp(line, "map_Ke", 4) && IS_SPACE(line[4]))
	{
		HandleTextureLine(line + 5, dir, curMaterial.EmissiveMap, false);
		return;
	}

	Log::Warning("Unknown token on line: '%s'!", line);
}
#pragma endregion

#pragma region Loading
void LoadMaterialLibraryFromFile(const string &dir, const char *name, ObjLoaderResult &result)
{
	/* Create reader to the file. */
	string path = dir;
	path += name;
	FileReader reader(path.toWide());

	/* Define current material. */
	ObjLoaderMaterial material;
	bool hasd = false, hastr = false;

	/* Read untill the end of the file. */
	while (reader.Peek() != EOF)
	{
		const string line = reader.ReadLine();

		/* Only handle line if it's not an empty line. */
		if (!line.empty()) HandleMtlLine(line.c_str(), dir, result, material, hasd, hastr);
	}

	/* Add last material to result if needed and release path. */
	if (!material.Name.empty()) result.Materials.emplace_back(material);
}

void LoadObjMtl(const string & path, ObjLoaderResult & result)
{
	/* Open obj file. */
	FileReader reader(path.toWide());
	const string dir = path.fileDirectory();

	/* Check if the file that is requested is an actual .obj file. */
	if (path.fileExtension().toUpper() != "OBJ") Log::Fatal("Specified model must be an .obj file not an '.%s' file", path.fileExtension());

	/* Defines current mesh and smoothing group. */
	ObjLoaderMesh shape;
	uint64 smoothingGroup;
	vector<string> loadedMaterialLibraries;

	/* Read untill the end of the file. */
	while (reader.Peek() != EOF)
	{
		const string line = reader.ReadLine();

		/* Only handle line if it's not an empty line. */
		if (!line.empty())
		{
			HandleObjLine(line.c_str(), dir, result, shape, smoothingGroup, loadedMaterialLibraries);
		}
	}

	/* Add last shape to result if needed and return. */
	if (!shape.Name.empty()) result.Shapes.emplace_back(shape);

	/* Shrink all result to minimize memory footprint. */
	result.Materials.shrink_to_fit();
	result.Normals.shrink_to_fit();
	result.Shapes.shrink_to_fit();
	result.TexCoords.shrink_to_fit();
	result.Vertices.shrink_to_fit();
}
#pragma endregion

void GenerateNodes(const ObjLoaderResult &input, PumIntermediate &result)
{
	const uint32 len = static_cast<uint32>(input.Shapes.size());
	result.Nodes.reserve(len);

	for (uint32 i = 0; i < len; i++)
	{
		pum_node node;
		node.SetMesh(i);
		result.Nodes.emplace_back(node);
	}
}

void CopyGeometry(const ObjLoaderResult &input, PumIntermediate &result)
{
	/* Reserve the amount of meshes needed. */
	result.Geometry.reserve(input.Shapes.size());

	/* 
	Make an educated guess for the size of the data buffer.
	Assumes all shapes will have normals and texture coordinates.
	*/
	size_t guessSize = 0;
	for (const ObjLoaderMesh &shape : input.Shapes)
	{
		guessSize += shape.Indices.size() * (sizeof(Vector3) + sizeof(Vector3) + sizeof(Vector2));
	}

	/* Pre allocate a buffer ans start copying the data. */
	result.Data.EnsureCapacity(guessSize);
	for (const ObjLoaderMesh &shape : input.Shapes)
	{
		/* Initialize the current shape's mesh. */
		pum_mesh mesh;
		mesh.Identifier = shape.Name.toUTF32();
		mesh.Topology = 3;	// Triangles
		if (shape.Material != -1) mesh.SetMaterial(static_cast<uint32>(shape.Material));

		/* Copy the vertex data. */
		mesh.VertexViewStart = result.Data.GetSize();
		bool firstVrtx = true;

		for (const ObjLoaderVertex &vrtx : shape.Indices)
		{
			const Vector3 pos = input.Vertices[vrtx.Vertex];

			if (firstVrtx)
			{
				firstVrtx = false;
				mesh.Bounds.LowerBound = pos;
				mesh.Bounds.UpperBound = pos;
			}
			else mesh.Bounds = mesh.Bounds.Merge(pos);

			result.Data.Write(pos);

			if (vrtx.Normal != -1)
			{
				mesh.HasNormals = true;
				result.Data.Write(input.Normals[vrtx.Normal]);
			}
			else if (mesh.HasNormals) Log::Fatal("Normals cannot be present on a subset of faces, either the shape contains no normals or all of the faces do!");

			if (vrtx.TexCoord != -1)
			{
				mesh.HasTextureUvs = true;
				result.Data.Write(input.TexCoords[vrtx.TexCoord]);
			}
			else if (mesh.HasTextureUvs) Log::Fatal("Texture coordinates cannot be present on a subset of faces, either the shape contains no texture coordinates or all of the faces do!");
		}

		/* Set the final buffer size and push it to the result. */
		mesh.VertexViewSize = result.Data.GetSize() - mesh.VertexViewStart;
		result.Geometry.emplace_back(mesh);
	}
}

uint32 ParseTexture(const ObjLoaderTextureMap &input, PumIntermediate &result)
{
	pum_texture texture;
	texture.Identifier = input.Path.toUTF32();
	texture.UsesLinearMagnification = true;
	texture.UsesLinearMinification = true;
	texture.UsesLinearMipmapMode = true;
	result.Textures.emplace_back(texture);

	return static_cast<uint32>(result.Textures.size() - 1);
}

void ParseMaterialsAndTextures(const ObjLoaderResult &input, PumIntermediate &result)
{
	result.Materials.reserve(input.Materials.size());
	for (const ObjLoaderMaterial &cur : input.Materials)
	{
		/* Handle factor data. */
		pum_material material;
		material.Identifier = cur.Name.toUTF32();
		material.DiffuseFactor = cur.Diffuse;
		material.SpecularFactor = cur.Specular;
		material.EmissiveFactor = cur.Emissive;
		material.Glossiness = cur.Glossiness;
		material.SpecularPower = log(cur.HighlightExponent);	// The value is presumably premultiplied as is normally scales from 0 to 1000
		if (material.EmissiveFactor != Color::Black()) material.EmissiveInternsity = 1.0f;
		material.AlphaMode = 1;
		material.AlphaThreshold = 0.1f;

		/* Handle optional texture maps. */
		if (!cur.AlbedoMap.Path.empty()) material.SetDiffuseTexture(ParseTexture(cur.AlbedoMap, result));
		if (!cur.SpecularMap.Path.empty()) material.SetSpecGlossTexture(ParseTexture(cur.SpecularMap, result));
		if (!cur.BumpMap.Path.empty()) material.SetNormalTexture(ParseTexture(cur.BumpMap, result));
		if (!cur.EmissiveMap.Path.empty()) material.SetEmissiveTexture(ParseTexture(cur.EmissiveMap, result));

		result.Materials.emplace_back(material);
	}
}

void ObjToPum(ObjLoaderResult & input, PumIntermediate & result)
{
	/* We need to generate nodes for OBJ files as they don't have a concept of them, so we just create a node per mesh. */
	GenerateNodes(input, result);
	CopyGeometry(input, result);

	/* All the geometry has been copied at this point so we can clear the vectors to save on memory for larger models. */
	input.Vertices.clear();
	input.Normals.clear();
	input.TexCoords.clear();
	input.Shapes.clear();

	/* Simply copy over the materials and texture and then clear this vector as well. */
	ParseMaterialsAndTextures(input, result);
	input.Materials.clear();
}