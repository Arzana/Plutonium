#pragma once
#include "Core\Math\Constants.h"
#include "DelegateObservers.h"
#include "Core\SafeMemory.h"

namespace Plutonium
{
	/* Provides an interface for an event subscriber. */
	template <typename _STy, typename ... _ArgTy>
	struct EventSubscriber
	{
	public:
		/* Defines a function style delegate. */
		using HandlerFuncType = void(*)(_In_ const _STy *sender, _In_ _ArgTy ... args);
		/* Defines a method style delegate. */
		template <typename _CTy>
		using HandlerMethodType = void(_CTy::*)(_In_ const _STy *sender, _In_ _ArgTy ... args);

		/* Initializes a new instance of an event subscriber. */
		EventSubscriber(HandlerFuncType func)
		{
			id = ComputeHash(func);
			hndlr = new DelegateFunc<_STy, _ArgTy...>(func);
		}

		/* Initializes a new instance of an event subscriber. */
		template <typename _CTy>
		EventSubscriber(_CTy *cnt, HandlerMethodType<_CTy> func)
		{
			id = ComputeHash(cnt, func);
			hndlr = new DelegateMethod<_STy, _CTy, _ArgTy...>(cnt, func);
		}

		EventSubscriber(_In_ const EventSubscriber<_STy, _ArgTy...> &value) = delete;
		EventSubscriber(_In_ EventSubscriber<_STy, _ArgTy...> &&value) = delete;

		/* Releases the resources of the event subscriber. */
		~EventSubscriber(void) noexcept
		{
			delete_s(hndlr);
		}

		_Check_return_ EventSubscriber& operator =(_In_ const EventSubscriber &other) = delete;
		_Check_return_ EventSubscriber& operator =(_In_ EventSubscriber &&other) = delete;

		/* Checks whether the subscriber has the same ID. */
		_Check_return_ inline bool operator ==(int64 id) const
		{
			return this->id == id;
		}

		/* Checks whether the subscriber has a different ID. */
		_Check_return_ inline bool operator !=(int64 id) const
		{
			return this->id != id;
		}

		/* Gets an ID that can be used to compare event subscribers without creating one. */
		_Check_return_ static inline int64 CreateComparableID(_In_ HandlerFuncType func)
		{
			return ComputeHash(func);
		}

		/* Gets an ID that can be used to compare subscribers without creating one. */
		template <typename _CTy>
		_Check_return_ static inline int64 CreateComparableID(_In_ const _CTy *cnt, _In_ HandlerMethodType<_CTy> func)
		{
			return ComputeHash(cnt, func);
		}

		/* Gets the ID of this subscriber. */
		_Check_return_ inline int64 GetID(void) const
		{
			return id;
		}

		/* Handles the post call from the parent event. */
		inline void HandlePost(_In_ const _STy *sender, _In_ _ArgTy ... args)
		{
			hndlr->Invoke(sender, args...);
		}

	private:
		int64 id;
		DelegateBase<_STy, _ArgTy...> *hndlr;

		static int64 ComputeHash(const void *arg1, const void *arg2)
		{
			int64 first = static_cast<int64>(reinterpret_cast<intptr_t>(arg1));
			int64 second = static_cast<int64>(reinterpret_cast<intptr_t>(arg2));
			return (first << 32) | second;
		}

		static int64 ComputeHash(HandlerFuncType func)
		{
			return ComputeHash(nullptr, void_ptr(func));
		}

		template <typename _CTy>
		static int64 ComputeHash(const _CTy *cnt, HandlerMethodType<_CTy> func)
		{
			return ComputeHash(void_ptr(cnt), (const void*&)func);
		}
	};
}