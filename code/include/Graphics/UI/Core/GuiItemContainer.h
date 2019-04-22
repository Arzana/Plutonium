#pragma once
#include "GuiItem.h"

namespace Pu
{
	/* Defines an object that owns UI items. */
	class GuiItemContainer
		: public Component
	{
	public:
		/* Initializes a new instance of a UI item container. */
		GuiItemContainer(_In_ Application &app);
		GuiItemContainer(_In_ const GuiItemContainer&) = delete;
		/* Move constructor. */
		GuiItemContainer(_In_ GuiItemContainer &&value);
		/* Releases the UI items owned by the container. */
		virtual ~GuiItemContainer(void);

		_Check_return_ GuiItemContainer& operator =(_In_ const GuiItemContainer&) = delete;
		_Check_return_ GuiItemContainer& operator =(_In_ GuiItemContainer&&) = delete;

		/* Gets the amount of controls handled by this container. */
		_Check_return_ inline size_t GetControlCount(void)
		{
			return controls.size();
		}

		/* Gets a type specific named control present in this container. */
		template <typename item_t>
		_Check_return_ inline item_t& GetControl(_In_ const string &name)
		{
			return dynamic_cast<item_t&>(GetControl(name));
		}

		/* Gets a type specific named control present in this container. */
		template <typename item_t>
		_Check_return_ inline const item_t& GetControl(_In_ const string &name) const
		{
			return dynamic_cast<item_t&>(GetControl(name));
		}

		/* Gets the UI item with the specified name. */
		_Check_return_ GuiItem& GetControl(_In_ const string &name);
		/* Gets the UI item with the specified name. */
		_Check_return_ const GuiItem& GetControl(_In_ const string &name) const;
		/* Renders the underlying UI items. */
		virtual void Render(_In_ GuiItemRenderer &renderer) const;
		/* Adds any UI item to the container (the container takes ownership). */
		void AddItem(_In_ GuiItem &item);
		/* Marks the specified UI item for delete, this will delete the item in the next update cycle. */
		void MarkForDelete(_In_ GuiItem &item);

	protected:
		/* Initializes the underlying UI items. */
		virtual void Initialize(void) override;
		/* Updates the underlying UI items. */
		virtual void Update(float dt) override;
		/* Finalizes the underlying UI items. */
		virtual void Finalize(void) override;
		/* Removes focus to all UI items in the container except the one specified. */
		void LoseFocusExceptOne(_In_ const GuiItem &exception);

		/* Gets the UI item at the specified index. */
		_Check_return_ inline GuiItem& GetControlAt(_In_ size_t idx)
		{
			return *std::get<1>(controls.at(idx));
		}

		/* Gets the UI item at the specified index. */
		_Check_return_ inline const GuiItem& GetControlAt(_In_ size_t idx) const
		{
			return *std::get<1>(controls.at(idx));
		}

	private:
		vector<std::pair<bool, GuiItem*>> controls;

		void RemoveItem(GuiItem &item);
	};
}