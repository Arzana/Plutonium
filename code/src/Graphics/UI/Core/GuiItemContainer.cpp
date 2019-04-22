#include "Graphics/UI/Core/GuiItemContainer.h"

Pu::GuiItemContainer::GuiItemContainer(Application & app)
	: Component(app)
{}

Pu::GuiItemContainer::GuiItemContainer(GuiItemContainer && value)
	: Component(std::move(value)), controls(std::move(value.controls))
{}

Pu::GuiItemContainer::~GuiItemContainer(void)
{
	for (auto[marked, item] : controls)
	{
		delete item;
	}

	controls.clear();
}

Pu::GuiItem& Pu::GuiItemContainer::GetControl(const string & name)
{
	for (auto[marked, item] : controls)
	{
		if (name == item->name) return *item;
	}

	Log::Fatal("Unable to find control with name '%s'!", name.c_str());
}

const Pu::GuiItem& Pu::GuiItemContainer::GetControl(const string & name) const
{
	for (const auto[marked, item] : controls)
	{
		if (name == item->name) return *item;
	}

	Log::Fatal("Unable to find control with name '%s'!", name.c_str());
}

void Pu::GuiItemContainer::Render(GuiItemRenderer & renderer) const
{
	for (const auto[marked, item] : controls)
	{
		item->Render(renderer);
	}
}

void Pu::GuiItemContainer::AddItem(GuiItem & item)
{
	/* Make sure the item can only be in one container at the same time. */
	if (item.container) item.container->RemoveItem(item);
	item.container = this;

	controls.emplace_back(std::make_pair(false, &item));
}

void Pu::GuiItemContainer::MarkForDelete(GuiItem & item)
{
	for (auto &[marked, cur] : controls)
	{
		if (&item == cur)
		{
			marked = true;
			return;
		}
	}

	Log::Warning("Attempting to mark unknown UI item '%s' for deletion!", item.name.c_str());
}

void Pu::GuiItemContainer::Initialize(void)
{
	Component::Initialize();
	for (auto[marked, item] : controls) item->Initialize();
}

void Pu::GuiItemContainer::Update(float dt)
{
	/* Only update is the contrianer is active. */
	if (IsEnabled())
	{
		for (size_t i = 0; i < controls.size();)
		{
			auto[marked, item] = controls[i];

			/* Delete the control if needed. */
			if (marked)
			{
				delete item;
				controls.removeAt(i);
				continue;
			}
			else i++;

			item->Update(dt);
		}
	}
}

void Pu::GuiItemContainer::Finalize(void)
{
	Component::Finalize();
	for (auto[marked, item] : controls) item->Finalize();
}

void Pu::GuiItemContainer::LoseFocusExceptOne(const GuiItem & exception)
{
	for (auto[marked, item] : controls)
	{
		if (&exception != item) item->ApplyFocus(false);
	}
}

void Pu::GuiItemContainer::RemoveItem(GuiItem & item)
{
	for (size_t i = 0; i < controls.size(); i++)
	{
		if (std::get<1>(controls[i]) == &item)
		{
			controls.removeAt(i);
			return;
		}
	}

	Log::Warning("UI item '%s' was not found in old container when moving ownership!", item.name.c_str());
}