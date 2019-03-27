#include "Components/Component.h"
#include "Application.h"

Pu::Component::Component(Application & app)
	: App(app), StateChanged("ComponentStateChanged"),
	initialized(false), enabled(true), place(0)
{}

void Pu::Component::Enable(void)
{
	if (!enabled)
	{
		enabled = true;
		StateChanged.Post(*this);
	}
}

void Pu::Component::Disable(void)
{
	if (enabled)
	{
		enabled = false;
		StateChanged.Post(*this);
	}
}

void Pu::Component::SetUpdatePlace(int32 newPlace)
{
	Log::Warning("Setting the update place after initialization is not valid!");
	place = newPlace;
}

void Pu::Component::DoInitialize(void)
{
	/* Used to force the set of initialized to true. */
	Initialize();
	initialized = true;
}

bool Pu::Component::SortPredicate(const Component * first, const Component * second)
{
	if (first->place && !second->place) return true;
	if (!first->place && second->place) return false;
	return first->place < second->place;
}