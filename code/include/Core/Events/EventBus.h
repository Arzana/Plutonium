#pragma once
#include <vector>
#include "EventSubscriber.h"

namespace Plutonium
{
	/* provides an interface for a subscribable event. */
	template <typename _STy, typename ... _ArgTy>
	struct EventBus
	{
	public:
		/* Defines a function style delegate. */
		using HandlerFuncType = void(*)(const _STy *sender, _ArgTy ... args);
		/* Defines a method style delegate. */
		template <typename _CTy>
		using HandlerMethodType = void(_CTy::*)(const _STy *sender, _ArgTy ... args);
		/* The type of subscriber stored in this event. */
		using SubscriberType = typename EventSubscriber<_STy, _ArgTy...>;

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
			for (size_t i = 0; i < callbacks.size(); i++)
			{
				SubscriberType *cur = callbacks.at(i);
				delete_s(cur);
			}

			callbacks.clear();
		}

		_Check_return_ EventBus& operator =(_In_ const EventBus &other) = delete;
		_Check_return_ EventBus& operator =(_In_ EventBus &&other) = delete;

		/* Registers an event handler to this event. */
		void Add(_In_ HandlerFuncType func) const
		{
			SubscriberType *sub = new SubscriberType(func);
			callbacks.push_back(sub);
			LOG("Registered callback(%llx) to event %s.", sub->GetID(), name);
		}

		/* Registers an event handler to this event. */
		template <typename _CTy>
		void Add(_In_ _CTy *obj, _In_ HandlerMethodType<_CTy> func) const
		{
			SubscriberType *sub = new SubscriberType(obj, func);
			callbacks.push_back(sub);
			LOG("Registered callback(%llx) to event %s.", sub->GetID(), name);
		}

		/* Unregisters an event handler from this event. */
		void Remove(_In_ HandlerFuncType func) const
		{
			int64 id = SubscriberType::CreateComparableID(func);
			size_t result = UnRegisterCallback(id);
			LOG("Unregistered %zu, callback(s)(%llx) from event %s.", result, id, name);
		}

		/* Unregisters an event handler from this event. */
		template <typename _CTy>
		void Remove(_In_ _CTy *obj, _In_ HandlerMethodType<_CTy> func) const
		{
			int64 id = SubscriberType::CreateComparableID(obj, func);
			size_t result = UnRegisterCallback(id);
			LOG("Unregistered %zu, callback(s)(%llx) from event %s.", result, id, name);
		}

		/* Posts an event to all registered subscribers. */
		void Post(_In_ const _STy *sender, _In_ _ArgTy ... args)
		{
			for (size_t i = 0; i < callbacks.size(); i++)
			{
				callbacks.at(i)->HandlePost(sender, args...);
			}
		}

	private:
		mutable std::vector<SubscriberType*> callbacks;
#if defined(DEBUG)
		const char *name;
#endif

		size_t UnRegisterCallback(int64 id) const
		{
			size_t result = 0;

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

			return result;
		}
	};
}