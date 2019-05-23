#pragma once
#include "Core/Events/EventBus.h"
#include "Core/Events/ValueChangedEventArgs.h"

namespace Pu
{
	class Application;

	/* Defines a basic application component that needs to be updated at a specific interval. */
	class Component
	{
	public:
		/* Occurs when the component is either enabled or disabled. */
		EventBus<Component, ValueChangedEventArgs<bool>> StateChanged;

		/* Copy constructor. */
		Component(_In_ const Component &value);
		/* Move constructor. */
		Component(_In_ Component &&value);
		/* Releases the resources allocated by this component. */
		virtual ~Component(void) {}

		_Check_return_ Component& operator =(_In_ const Component&) = delete;
		_Check_return_ Component& operator =(_In_ Component&&) = delete;

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
		/* The application associated with this component. */
		Application *App;

		/* Initializes a new instance of a component. */
		Component(_In_ Application &app);

		/* Initializes the component. */
		virtual void Initialize(void) {}
		/* Updates the component. */
		virtual void Update(_In_ float dt) = 0;
		/* Finalizes the component. */
		virtual void Finalize(void) {}

	private:
		friend class Application;

		bool enabled, initialized;
		float accum, rate;
		int32 place;

		static bool SortPredicate(const Component *first, const Component *second);

		void DoInitialize(void);
	};
}