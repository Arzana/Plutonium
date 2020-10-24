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
	Pu::string Temp;							// -dbg (optional)
	Pu::string DisplayName;						// -dn (optional)
	ContentType Type;							// Generated (required)
	bool CreateTangents;						// -t (optional)
	bool RecalcTangents;						// -rt (optional)
	Pu::vector<Pu::string> AdditionalTextures;	// -at (optional)

	CLArgs(void)
		: Type(ContentType::Unknown), CreateTangents(false), RecalcTangents(false)
	{}

	CLArgs(const CLArgs&) = delete;
	CLArgs(CLArgs&&) = delete;

	CLArgs& operator =(const CLArgs&) = delete;
	CLArgs& operator =(CLArgs&&) = delete;
};