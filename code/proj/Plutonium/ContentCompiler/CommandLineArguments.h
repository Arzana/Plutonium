#pragma once
#include <Core/Threading/Tasks/Scheduler.h>

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
	bool BakeMeshes;							// -b (optional)
	bool RecalcTangents;						// -t (optional)
	Pu::vector<Pu::string> AdditionalTextures;	// -at (optional)
	Pu::TaskScheduler *Scheduluer;				// Just use a globally available scheduler, so we don't allocate it a few times.

	CLArgs(void)
		: Type(ContentType::Unknown), RecalcTangents(false), BakeMeshes(false)
	{
		Scheduluer = new Pu::TaskScheduler();
	}

	~CLArgs(void)
	{
		delete Scheduluer;
	}

	CLArgs(const CLArgs&) = delete;
	CLArgs(CLArgs&&) = delete;

	CLArgs& operator =(const CLArgs&) = delete;
	CLArgs& operator =(CLArgs&&) = delete;
};