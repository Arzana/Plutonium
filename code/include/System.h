#pragma once
#include "Core/Events/EventBus.h"
#include "Core/Events/ValueChangedEventArgs.h"

namespace Pu
{
	/* Defines a basic application component that needs to be updated at a specific interval. */
	class System
	{
	public:
		/* Occurs when the component is either enabled or disabled. */
		EventBus<System, ValueChangedEventArgs<bool>> StateChanged;

		/* Copy constructor. */
		System(_In_ const System &value);
		/* Move constructor. */
		System(_In_ System &&value);
		/* Releases the resources allocated by this component. */
		virtual ~System(void) {}

		_Check_return_ System& operator =(_In_ const System&) = delete;
		_Check_return_ System& operator =(_In_ System&&) = delete;

		/* Enabled the component. */
		void Enable(void);
		/* Disables the component. */
		void Disable(void);
		/* Sets the prefered position in the update queue (0 means that the place doesn't matter). */
		void SetUpdatePlace(_In_ int32 newPlace);

		/* Gets whether this component is enabled. */
		_Check_return_ inline bool IsEnabled(void) const
		{
			return enabled;
		}

	protected:
		/* Initializes a new instance of a system. */
		System(void);

		/* Initializes the system. */
		virtual void Initialize(void) {}
		/* Updates the system. */
		virtual void Update(_In_ float dt) = 0;
		/* Finalizes the system. */
		virtual void Finalize(void) {}

	private:
		friend class Application;

		bool enabled, initialized;
		int32 place;

		static bool SortPredicate(const System *first, const System *second);

		void DoInitialize(void);
	};
}