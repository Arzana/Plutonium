#include "Components/Component.h"
#include "Application.h"

Pu::Component::Component(Application & app)
	: App(&app), StateChanged("ComponentStateChanged"),
	initialized(false), enabled(true), place(0)
{}

Pu::Component::Component(const Component & value)
	: App(value.App), StateChanged(value.StateChanged),
	initialized(value.initialized), enabled(value.enabled), place(value.place)
{}

Pu::Component::Component(Component && value)
	: App(value.App), StateChanged(std::move(value.StateChanged)),
	initialized(value.initialized), enabled(value.enabled), place(value.place)
{
	value.initialized = false;
	value.enabled = false;
}

void Pu::Component::Enable(void)
{
	static ValueChangedEventArgs<bool> args(false, true);

	if (!enabled)
	{
		enabled = true;
		StateChanged.Post(*this, args);
	}
}

void Pu::Component::Disable(void)
{
	static ValueChangedEventArgs<bool> args(true, false);

	if (enabled)
	{
		enabled = false;
		StateChanged.Post(*this, args);
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