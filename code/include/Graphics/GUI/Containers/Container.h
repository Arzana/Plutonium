#pragma once
#include "Graphics\GUI\Core\GuiItem.h"

namespace Plutonium
{
	/* Defines a base object for GuiItem container types. */
	struct Container
	{
		/* Initializes a new instance of a container base. */
		Container(void);
		Container(_In_ const Container &value) = delete;
		Container(_In_ Container &&value) = delete;
		/* Releases the resources allocated by the container. */
		~Container(void);

		_Check_return_ Container& operator =(_In_ const Container &other) = delete;
		_Check_return_ Container& operator =(_In_ Container &&other) = delete;

		/* Gets a type specified named control present in this container. */
		template<typename _Ty>
		_Check_return_ inline _Ty* GetTypedControl(_In_ const char *name) const
		{
			return dynamic_cast<_Ty*>(GetControl(name));
		}

		/* Gets a named control present in this container. */
		_Check_return_ GuiItem* GetControl(_In_ const char *name) const;
		/* Check if any GuiItem has focus in this container. */
		_Check_return_ bool HasFocus(void) const;
		/* Adds any GuiItem to the container (this will be deleted by the container). */
		void AddItem(_In_ GuiItem *item);
		/* Marks the specified GuiItem for delete, this will delete the item in the next update cycle. */
		void MarkForDelete(_In_ const GuiItem *item);

	protected:
		/* Updates the underlying GuiItems. */
		virtual void Update(_In_ float dt);
		/* Renders the underlying GuiItems. */
		virtual void Render(GuiItemRenderer *renderer);

		/* Applies a focus of value false to all GuiItems in the container except the one specified. */
		void LoseFocusExceptOne(_In_ const GuiItem *exception);

		/* Gets the amount of controlls handled by this Container. */
		_Check_return_ inline size_t GetControlCount(void) const
		{
			return controlls.size();
		}

		/* Gets a control at the specified index. */
		_Check_return_ inline GuiItem* GetControllAt(_In_ size_t idx) const
		{
			ASSERT_IF(idx >= GetControlCount(), "Index out of range!");
			return std::get<1>(controlls.at(idx));
		}

	private:
		std::vector<std::tuple<bool, GuiItem*>> controlls;

		void RemoveItem(GuiItem *item);
	};
}