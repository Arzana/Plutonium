#pragma once
#include "Core/Collections/Vector.h"
#include "EventSubscriber.h"
#include "Config.h"
#include <mutex>

namespace Pu
{
	/* provides an interface for a subscribable event. */
	template <typename _STy, typename ... _ArgTy>
	class EventBus
	{
	public:
		/* The type of subscriber stored in this event. */
		using SubscriberType = typename EventSubscriber<_STy, _ArgTy...>;
		/* Defines a function style delegate. */
		using HandlerFuncType = typename SubscriberType::HandlerFuncType;
		/* Defines a method style delegate. */
		template <typename _CTy>
		using HandlerMethodType = typename SubscriberType::template HandlerMethodType<_CTy>;

		/* Initializes a new instance of an event (name is only used on debug mode). */
		EventBus(_In_ const char *name)
			: callbacks(), name(name)
		{}

		/* Initializes a new instance of an event bus as a copy of the specified bus. */
		EventBus(_In_ const EventBus &value)
			: name(value.name), callbacks(value.callbacks)
		{}

		/* Moves the specified subscriber instance to a new instance. */
		EventBus(_In_ EventBus &&value)
			: name(value.name)
		{
			callbacks = std::move(value.callbacks);

			value.name = "";
			value.callbacks.clear();
		}

		/* Copies the data from the specified bus to this bus. */
		_Check_return_ EventBus& operator =(_In_ const EventBus &other)
		{
			if (this != &other)
			{
				name = other.name;
				callbacks = vector<SubscriberType>(other.callbacks);
			}

			return *this;
		}

		/* Moves the data from the specified bus to this bus. */
		_Check_return_ EventBus& operator =(_In_ EventBus &&other)
		{
			if (this != &other)
			{
				name = other.name;
				callbacks = std::move(other.callbacks);

				other.name = "";
				other.callbacks.clear();
			}

			return *this;
		}

		/* Registers an event handler to this event. */
		void operator +=(_In_ HandlerFuncType func)
		{
			Add(func);
		}

		/* Registers an event handler to this event. */
		template <typename _LTy>
		void operator +=(_In_ const _LTy &lambda)
		{
			Add(lambda);
		}

		/* Unregisters an event handler from this event. */
		void operator -=(_In_ HandlerFuncType func)
		{
			Remove(func);
		}

		/* Registers an event handler to this event. */
		void Add(_In_ HandlerFuncType func) const
		{
			SubscriberType sub(func);
			lock.lock();
			callbacks.push_back(sub);
			lock.unlock();
			if constexpr (EventBusLogging) Log::Verbose("Registered callback(%llX) to event %s.", sub.GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename _CTy>
		void Add(_In_ _CTy &obj, _In_ HandlerMethodType<_CTy> func) const
		{
			SubscriberType sub(obj, func);
			lock.lock();
			callbacks.push_back(sub);
			lock.unlock();
			if constexpr (EventBusLogging) Log::Verbose("Registered callback(%llX) to event %s.", sub.GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename _LTy>
		void Add(_In_ const _LTy &lambda) const
		{
			SubscriberType sub(lambda);
			lock.lock();
			callbacks.push_back(sub);
			lock.unlock();
			if constexpr (EventBusLogging) Log::Verbose("Registered callback(lambda) to event %s.", name);
		}

		/* Unregisters an event handler from this event. */
		void Remove(_In_ HandlerFuncType func) const
		{
			const int64 id = SubscriberType::CreateComparableID(func);
			const size_t result = UnRegisterCallback(id);
			if constexpr (EventBusLogging) Log::Verbose("Unregistered %zu callback(s)(%llX) from event %s.", result, id, name);
		}

		/* Unregisters an event handler from this event. */
		template <typename _CTy>
		void Remove(_In_ _CTy &obj, _In_ HandlerMethodType<_CTy> func) const
		{
			const int64 id = SubscriberType::CreateComparableID(obj, func);
			const size_t result = UnRegisterCallback(id);
			if constexpr (EventBusLogging) Log::Verbose("Unregistered %zu callback(s)(%llX) from event %s.", result, id, name);
		}

		/* Posts an event to all registered subscribers. */
		void Post(_In_ _STy &sender, _In_ _ArgTy ... args)
		{
			lock.lock();
			for (size_t i = 0; i < callbacks.size(); i++) callbacks[i].HandlePost(sender, args...);
			lock.unlock();
		}

	private:
		mutable vector<SubscriberType> callbacks;
		mutable std::mutex lock;
		const char *name;

		size_t UnRegisterCallback(int64 id) const
		{
			lock.lock();
			const size_t result = callbacks.removeAll(id, [](const SubscriberType &element, const int64 &id) { return element == id; });
			lock.unlock();
			return result;
		}
	};
}