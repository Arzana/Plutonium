#pragma once
#include <Core/String.h>

enum class ContentType
{
	Unknown,
	PUM
};

struct CLArgs
{
	Pu::string Input;							// Last argument (required).
	Pu::string Output;							// -o (optional)
	Pu::string DisplayName;						// -dn (optional)
	ContentType Type = ContentType::Unknown;	// Generated (required).
	bool RecalcNormals;							// -n (optional)
	bool RecalcTangents;						// -t (optional)
	Pu::vector<Pu::string> AdditionalTextures;	// -at (optional)
};