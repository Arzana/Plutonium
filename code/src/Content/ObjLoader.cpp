#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"

using namespace Plutonium;
using namespace std;

Plutonium::ObjLoaderVertex::ObjLoaderVertex(void)
	: Vertex(-1), Normal(-1), TexCoord(-1)
{}

Plutonium::ObjLoaderMesh::ObjLoaderMesh(void)
	: Name(""), Indices(), VerticesPerFace(), Materials(), SmoothingGroups()
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

Plutonium::ObjLoaderResult::ObjLoaderResult(void)
	: Vertices(), Normals(), TexCoords(), Shapes(), Materials()
{}

void HandleLine(const char *line, ObjLoaderResult *result, ObjLoaderMesh *curMesh, ObjLoaderMaterial *curMaterial)
{

}

const ObjLoaderResult * Plutonium::_CrtLoadObjMtl(const char * path)
{
	/* Setup input and open obj file. */
	ObjLoaderResult *result = new ObjLoaderResult();
	FileReader reader(path);

	/* Defines current mesh and material. */
	ObjLoaderMesh shape;
	ObjLoaderMaterial material;

	/* Read untill the end of the file. */
	while (reader.Peek() != EOF)
	{
		const char *line = reader.ReadLine();
		HandleLine(line, result, &shape, &material);
		free_s(line);
	}

	/* Add last shape and material to result if needed and return. */
	if (strlen(shape.Name) > 0) result->Shapes.push_back(shape);
	if (strlen(material.Name) > 0) result->Materials.push_back(material);
	return result;
}