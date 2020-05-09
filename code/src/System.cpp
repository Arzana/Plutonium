#include "System.h"
#include "Application.h"

Pu::System::System(void)
	: StateChanged("ComponentStateChanged"),
	initialized(false), enabled(true), place(0)
{}

Pu::System::System(const System & value)
	: StateChanged(value.StateChanged),
	initialized(value.initialized), enabled(value.enabled), place(value.place)
{}

Pu::System::System(System && value)
	: StateChanged(std::move(value.StateChanged)),
	initialized(value.initialized), enabled(value.enabled), place(value.place)
{
	value.initialized = false;
	value.enabled = false;
}

void Pu::System::Enable(void)
{
	static ValueChangedEventArgs<bool> args(false, true);

	if (!enabled)
	{
		enabled = true;
		StateChanged.Post(*this, args);
	}
}

void Pu::System::Disable(void)
{
	static ValueChangedEventArgs<bool> args(true, false);

	if (enabled)
	{
		enabled = false;
		StateChanged.Post(*this, args);
	}
}

void Pu::System::SetUpdatePlace(int32 newPlace)
{
	Log::Warning("Setting the update place after initialization is not valid!");
	place = newPlace;
}

void Pu::System::DoInitialize(void)
{
	/* Used to force the set of initialized to true. */
	Initialize();
	initialized = true;
}

bool Pu::System::SortPredicate(const System * first, const System * second)
{
	if (first->place && !second->place) return true;
	if (!first->place && second->place) return false;
	return first->place < second->place;
}