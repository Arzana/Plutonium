#include "Components\GameComponent.h"
#include "Core\Diagnostics\Logging.h"

Plutonium::GameComponent::GameComponent(Game * game)
	: game(game), initialized(false), enabled(true), ActiveDuringLoad(false),
	place(-1), INIT_BUS(StateChanged)
{}

void Plutonium::GameComponent::Enable(void)
{
	if (!enabled)
	{
		enabled = true;
		StateChanged.Post(this, EventArgs());
	}
}

void Plutonium::GameComponent::Disable(void)
{
	if (enabled)
	{
		enabled = false;
		StateChanged.Post(this, EventArgs());
	}
}

void Plutonium::GameComponent::SetUpdatePlace(int place)
{
	LOG_WAR_IF(initialized, "Cannot set the update place after the component is loaded!");
	place = place;
}

void Plutonium::GameComponent::Initialize(void)
{
	initialized = true;
}

void Plutonium::GameComponent::Update(float dt)
{}

void Plutonium::GameComponent::Render(float dt)
{}

void Plutonium::GameComponent::Finalize(void)
{
	initialized = false;
}
