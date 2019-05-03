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
	Pu::string DisplayName;						// -n (optional)
	ContentType Type = ContentType::Unknown;	// -t (optional)
	bool IsValid = true;
};