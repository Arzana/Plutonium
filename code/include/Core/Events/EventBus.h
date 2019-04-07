#pragma once
#include "Core/Collections/Vector.h"
#include "DelegateObsevers.h"
#include "Config.h"
#include <mutex>

namespace Pu
{
	/* provides an interface for a subscribable event. */
	template <typename sender_t, typename ... argument_t>
	class EventBus
	{
	public:
		/* The type of subscriber stored in this event. */
		using subscriber_t = DelegateBase<sender_t, argument_t...>*;
		/* The type of function subscriber used in this event. */
		using subscriberFunc_t = DelegateFunc<sender_t, argument_t...>;
		/* The type of method subscriber used in this event. */
		template <typename container_t>
		using subscriberMethod_t = DelegateMethod<sender_t, container_t, argument_t...>;
		/* The type of lambda subscriber used in this event. */
		template <typename lambda_t>
		using subscriberLambda_t = DelegateLambda<sender_t, lambda_t, argument_t...>;


		/* Defines a function style delegate. */
		using handlerFunc_t = typename subscriberFunc_t::handler_t;
		/* Defines a method style delegate. */
		template <typename container_t>
		using handlerMethod_t = typename subscriberMethod_t<container_t>::handler_t;

		/* Initializes a new instance of an event (name is only used on debug mode). */
		EventBus(_In_ const char *name)
			: callbacks(), name(name)
		{}

		/* Initializes a new instance of an event bus as a copy of the specified bus. */
		EventBus(_In_ const EventBus &value)
			: name(value.name)
		{
			Copy(value);
		}

		/* Moves the specified subscriber instance to a new instance. */
		EventBus(_In_ EventBus &&value)
			: name(value.name), callbacks(std::move(value.callbacks))
		{
			value.name = "";
			value.callbacks.clear();
		}

		/* Releases the resources allocated by the event bus. */
		~EventBus(void)
		{
			Destroy();
		}

		/* Copies the data from the specified bus to this bus. */
		_Check_return_ EventBus& operator =(_In_ const EventBus &other)
		{
			if (this != &other)
			{
				Destroy();
				name = other.name;
				Copy(other);
			}

			return *this;
		}

		/* Moves the data from the specified bus to this bus. */
		_Check_return_ EventBus& operator =(_In_ EventBus &&other)
		{
			if (this != &other)
			{
				Destroy();

				name = other.name;
				callbacks = std::move(other.callbacks);

				other.name = "";
				other.callbacks.clear();
			}

			return *this;
		}

		/* Registers an event handler to this event. */
		void operator +=(_In_ handlerFunc_t func) const
		{
			Add(func);
		}

		/* Registers an event handler to this event. */
		template <typename lambda_t>
		void operator +=(_In_ const lambda_t &lambda) const
		{
			Add(lambda);
		}

		/* Unregisters an event handler from this event. */
		void operator -=(_In_ handlerFunc_t func) const
		{
			Remove(func);
		}

		/* Registers an event handler to this event. */
		void Add(_In_ handlerFunc_t func) const
		{
			subscriberFunc_t *sub = new subscriberFunc_t(func);
			lock.lock();
			callbacks.emplace_back(sub);
			lock.unlock();
			if constexpr (EventBusLogging) Log::Verbose("Registered callback(%llX) to event %s.", sub->GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename container_t>
		void Add(_In_ container_t &obj, _In_ handlerMethod_t<container_t> func) const
		{
			subscriberMethod_t<container_t> *sub = new subscriberMethod_t<container_t>(obj, func);
			lock.lock();
			callbacks.emplace_back(sub);
			lock.unlock();
			if constexpr (EventBusLogging) Log::Verbose("Registered callback(%llX) to event %s.", sub->GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename lambda_t>
		void Add(_In_ const lambda_t &lambda) const
		{
			subscriberLambda_t<lambda_t> *sub = new subscriberLambda_t<lambda_t>(lambda);
			lock.lock();
			callbacks.emplace_back(sub);
			lock.unlock();
			if constexpr (EventBusLogging) Log::Verbose("Registered callback(lambda) to event %s.", name);
		}

		/* Unregisters an event handler from this event. */
		void Remove(_In_ handlerFunc_t func) const
		{
			const int64 id = subscriberFunc_t::ComputeHash(func);
			const size_t result = UnRegisterCallback(id);
			if constexpr (EventBusLogging) Log::Verbose("Unregistered %zu callback(s)(%llX) from event %s.", result, id, name);
		}

		/* Unregisters an event handler from this event. */
		template <typename container_t>
		void Remove(_In_ container_t &obj, _In_ handlerMethod_t<container_t> func) const
		{
			const int64 id = subscriberMethod_t<container_t>::ComputeHash(obj, func);
			const size_t result = UnRegisterCallback(id);
			if constexpr (EventBusLogging) Log::Verbose("Unregistered %zu callback(s)(%llX) from event %s.", result, id, name);
		}

		/* Posts an event to all registered subscribers. */
		void Post(_In_ sender_t &sender, _In_ argument_t ... args)
		{
			lock.lock();
			for (subscriber_t cur : callbacks) cur->Invoke(sender, args...);
			lock.unlock();
		}

	private:
		mutable vector<subscriber_t> callbacks;
		mutable std::mutex lock;
		const char *name;

		size_t UnRegisterCallback(int64 id) const
		{
			lock.lock();
			const size_t result = callbacks.removeAll([id](const subscriber_t element) { return *element == id; });
			lock.unlock();
			return result;
		}

		void Copy(const EventBus &other)
		{
			callbacks.reserve(other.callbacks.size());
			for (const subscriber_t cur : other.callbacks) callbacks.emplace_back(cur->Copy());
		}

		void Destroy()
		{
			for (const subscriber_t cur : callbacks) delete cur;
		}
	};
}