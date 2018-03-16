#pragma once
#include <Graphics\Models\AnimationInfo.h>
#include <Core\Diagnostics\Logging.h>

inline void InitKnight(const char *Name, Plutonium::PlayBackFlags &flags, float &fps)
{
	static bool logCullWar = true;
	if (logCullWar)
	{
		logCullWar = false;
		LOG_WAR("The knight model cannot work with face culling enabled!");
	}

	if (!strcmp(Name, "stand"))
	{
		flags = Plutonium::PlayBackFlags::DefaultLoop;
		fps = 10;
	}
	else if (!strcmp(Name, "run"))
	{
		flags = Plutonium::PlayBackFlags::DefaultLoop;
		fps = 5;
	}
	else if (!strcmp(Name, "attack"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 7;
	}
	else if (!strcmp(Name, "pain1"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 8;
	}
	else if (!strcmp(Name, "pain2"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 7;
	}
	else if (!strcmp(Name, "pain3"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 8;
	}
	else if (!strcmp(Name, "jump"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 10;
	}
	else if (!strcmp(Name, "flip"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 7;
	}
	else if (!strcmp(Name, "salute"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 8;
	}
	else if (!strcmp(Name, "taunt"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 4;
	}
	else if (!strcmp(Name, "wave"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 6;
	}
	else if (!strcmp(Name, "point"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 7;
	}
	else if (!strcmp(Name, "death1"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 11;
	}
	else if (!strcmp(Name, "death2"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 8;
	}
	else if (!strcmp(Name, "death3"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 5;
	}
	else if (!strcmp(Name, "crstnd"))
	{
		flags = Plutonium::PlayBackFlags::DefaultLoop;
		fps = 8;
	}
	else if (!strcmp(Name, "crwalk"))
	{
		flags = Plutonium::PlayBackFlags::DefaultLoop;
		fps = 3;
	}
	else if (!strcmp(Name, "crattk"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 9;
	}
	else if (!strcmp(Name, "crpain"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 10;
	}
	else if (!strcmp(Name, "crdeath"))
	{
		flags = Plutonium::PlayBackFlags::Default;
		fps = 9;
	}
	else LOG_WAR("Unset Knight animation: %s!", Name);
}