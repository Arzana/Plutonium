#include "Components\GameComponent.h"
#include "Core\Diagnostics\Logging.h"

Plutonium::GameComponent::GameComponent(Game * game)
	: game(game), initialized(false), enabled(true), place(-1), StateChanged("GameComponentStateChanged")
{}

void Plutonium::GameComponent::Enable(void)
{
	enabled = true;
	StateChanged.Post(this, EventArgs());
}

void Plutonium::GameComponent::Disable(void)
{
	enabled = false;
	StateChanged.Post(this, EventArgs());
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

void Plutonium::GameComponent::Render(float dt)
{}

void Plutonium::GameComponent::Finalize(void)
{
	initialized = false;
}
