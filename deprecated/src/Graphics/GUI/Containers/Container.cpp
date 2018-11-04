#include "Graphics\GUI\Containers\Container.h"
#include "Core\StringFunctions.h"

Plutonium::Container::Container(void)
{}

Plutonium::Container::~Container(void)
{
	for (size_t i = 0; i < controlls.size(); i++) delete_s(std::get<1>(controlls.at(i)));
	controlls.clear();
}

Plutonium::GuiItem * Plutonium::Container::GetControl(const char * name) const
{
	for (size_t i = 0; i < controlls.size(); i++)
	{
		GuiItem *cur = std::get<1>(controlls.at(i));
		if (eqlstr(cur->GetName(), name)) return cur;
	}

	LOG_WAR("Unable to find control '%s'!", name);
	return nullptr;
}

bool Plutonium::Container::HasFocus(void) const
{
	for (size_t i = 0; i < controlls.size(); i++)
	{
		if (std::get<1>(controlls.at(i))->IsFocused()) return true;
	}

	return false;
}

void Plutonium::Container::AddItem(GuiItem * item)
{
	/* Make sure the item can only be part of one container to avoid extra updates and draw calls. */
	if (item->container) item->container->RemoveItem(item);
	item->container = this;

	controlls.push_back(std::make_tuple(false, item));
}

void Plutonium::Container::MarkForDelete(const GuiItem * item)
{
	for (size_t i = 0; i < controlls.size(); i++)
	{
		std::tuple<bool, GuiItem*> &cur = controlls.at(i);
		if (std::get<1>(cur) == item)
		{
			std::get<0>(cur) = true;
			return;
		}
	}

	LOG_WAR("Attempting to mark an unknown GuiItem for deletion!");
}

void Plutonium::Container::Update(float dt)
{
	/* Update all GuiItems. */
	for (size_t i = 0; i < controlls.size(); i++)
	{
		std::tuple<bool, GuiItem*> &cur = controlls.at(i);

		/* Delete the control if it's marked for delete. */
		if (std::get<0>(cur))
		{
			delete_s(std::get<1>(cur));
			controlls.erase(controlls.begin() + i--);
			continue;
		}

		std::get<1>(cur)->Update(dt);
	}
}

void Plutonium::Container::Render(GuiItemRenderer * renderer)
{
	/* Render all GuiItems. */
	for (size_t i = 0; i < controlls.size(); i++)
	{
		std::get<1>(controlls.at(i))->Draw(renderer);
	}
}

void Plutonium::Container::LoseFocusExceptOne(const GuiItem * exception)
{
	for (size_t i = 0; i < controlls.size(); i++)
	{
		GuiItem *cur = std::get<1>(controlls.at(i));
		if (cur != exception) cur->ApplyFocus(false);
	}
}

void Plutonium::Container::RemoveItem(GuiItem * item)
{
	for (size_t i = 0; i < controlls.size(); i++)
	{
		if (std::get<1>(controlls.at(i)) == item)
		{
			controlls.erase(controlls.begin() + i);
			return;
		}
	}
}