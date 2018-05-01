#pragma once
#include <vector>
#include <mutex>
#include "EventSubscriber.h"
#include "Core\Diagnostics\ReflectNames.h"

/* Initializes an eventbus with it's own name as the debug display name. */
#define INIT_BUS(bus)		bus(NAMEOF(bus))

namespace Plutonium
{
	/* provides an interface for a subscribable event. */
	template <typename _STy, typename ... _ArgTy>
	struct EventBus
	{
	public:
		/* The type of subscriber stored in this event. */
		using SubscriberType = typename EventSubscriber<_STy, _ArgTy...>;
		/* Defines a function style delegate. */
		using HandlerFuncType = typename SubscriberType::HandlerFuncType;
		/* Defines a method style delegate. */
		template <typename _CTy>
		using HandlerMethodType = void(_CTy::*)(const _STy *sender, _ArgTy ... args);

		/* Initializes a new instance of an event (name is only used on debug mode). */
		EventBus(_In_ const char *name)
			: callbacks()
#if defined(DEBUG)
			, name(name)
#endif
		{}

		EventBus(_In_ const EventBus &value) = delete;
		EventBus(_In_ EventBus &&value) = delete;

		/* Releases the resources of this event. */
		~EventBus(void) noexcept
		{
			lock.lock();
			for (size_t i = 0; i < callbacks.size(); i++) delete_s(callbacks.at(i));
			callbacks.clear();
			lock.unlock();
		}

		_Check_return_ EventBus& operator =(_In_ const EventBus &other) = delete;
		_Check_return_ EventBus& operator =(_In_ EventBus &&other) = delete;

		/* Registers an event handler to this event. */
		void Add(_In_ HandlerFuncType func) const
		{
			SubscriberType *sub = new SubscriberType(func);
			lock.lock();
			callbacks.push_back(sub);
			lock.unlock();
			LOG("Registered callback(%llX) to event %s.", sub->GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename _CTy>
		void Add(_In_ _CTy *obj, _In_ HandlerMethodType<_CTy> func) const
		{
			SubscriberType *sub = new SubscriberType(obj, func);
			lock.lock();
			callbacks.push_back(sub);
			lock.unlock();
			LOG("Registered callback(%llX) to event %s.", sub->GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename _LTy>
		void Add(_In_ const _LTy &lambda) const
		{
			SubscriberType *sub = new SubscriberType(lambda);
			lock.lock();
			callbacks.push_back(sub);
			lock.unlock();
			LOG("Registered callback(lambda) to event %s.", name);
		}

		/* Unregisters an event handler from this event. */
		void Remove(_In_ HandlerFuncType func) const
		{
			int64 id = SubscriberType::CreateComparableID(func);
			size_t result = UnRegisterCallback(id);
			LOG("Unregistered %zu callback(s)(%llX) from event %s.", result, id, name);
		}

		/* Unregisters an event handler from this event. */
		template <typename _CTy>
		void Remove(_In_ _CTy *obj, _In_ HandlerMethodType<_CTy> func) const
		{
			int64 id = SubscriberType::CreateComparableID(obj, func);
			size_t result = UnRegisterCallback(id);
			LOG("Unregistered %zu callback(s)(%llX) from event %s.", result, id, name);
		}

		/* Posts an event to all registered subscribers. */
		void Post(_In_ const _STy *sender, _In_ _ArgTy ... args)
		{
			lock.lock();
			for (size_t i = 0; i < callbacks.size(); i++) callbacks.at(i)->HandlePost(sender, args...);
			lock.unlock();
		}

	private:
		mutable std::vector<SubscriberType*> callbacks;
		mutable std::mutex lock;
#if defined(DEBUG)
		const char *name;
#endif

		size_t UnRegisterCallback(int64 id) const
		{
			size_t result = 0;

			lock.lock();
			for (size_t i = 0; i < callbacks.size(); i++)
			{
				SubscriberType *cur = callbacks.at(i);
				if (*cur == id)
				{
					delete_s(cur);
					callbacks.erase(callbacks.begin() + i);

					--i;
					++result;
				}
			}

			lock.unlock();
			return result;
		}
	};
}