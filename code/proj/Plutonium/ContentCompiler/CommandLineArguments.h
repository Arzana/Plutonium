#pragma once
#include <Core/String.h>

enum class ContentType
{
	Unknown,
	PUM
};

struct CLArgs
{
	Pu::string Input;							// Last argument (required)
	Pu::string Output;							// -o (optional)
	Pu::string DisplayName;						// -dn (optional)
	ContentType Type;							// Generated (required)
	bool RecalcNormals;							// -n (optional) (cannot be active at the same time as reorder faces)
	bool RecalcTangents;						// -t (optional)
	bool ReorderFaces;							// -rf (optional) (cannot be active at the same time as recalc normals)
	Pu::vector<Pu::string> AdditionalTextures;	// -at (optional)

	CLArgs(void)
		: Type(ContentType::Unknown), RecalcNormals(false),
		RecalcTangents(false), ReorderFaces(false)
	{}
};