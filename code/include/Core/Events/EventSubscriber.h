#pragma once
#include "Core\Math\Constants.h"
#include "DelegateObservers.h"
#include "Core\SafeMemory.h"

namespace Plutonium
{
	/* Provides an interface for an event subscriber. */
	template <typename _STy, typename ... _ArgTy>
	class EventSubscriber
	{
	public:
		/* Defines a function style delegate. */
		using HandlerFuncType = typename DelegateFunc<_STy, _ArgTy...>::HandlerType;
		/* Defines a method style delegate. */
		template <typename _CTy>
		using HandlerMethodType = void(_CTy::*)(_In_ const _STy *sender, _In_ _ArgTy ... args);

		/* Initializes an empty instance of an event subscriber. */
		EventSubscriber(void)
			: id(0), hndlr(nullptr)
		{}

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

		/* Initializes a new instance of an event subscriber. */
		template <typename _LTy>
		EventSubscriber(const _LTy &lambda)
		{
			id = ComputeHash(reinterpret_cast<const _LTy*>(nullptr), reinterpret_cast<HandlerMethodType<_LTy>>(&_LTy::operator()));
			hndlr = new DelegateLambda<_STy, _LTy, _ArgTy...>(lambda);
		}

		/* Initializes a new instance of an event subscriber as a copy of the specified subscriber. */
		EventSubscriber(_In_ const EventSubscriber<_STy, _ArgTy...> &value)
			: id(value.id)
		{
			hndlr = value.hndlr->Copy();
		}

		/* Moves the specified subscriber instance to a new instance. */
		EventSubscriber(_In_ EventSubscriber<_STy, _ArgTy...> &&value)
			: id(value.id)
		{
			hndlr = value.hndlr;

			value.id = 0;
			value.hndlr = nullptr;
		}

		/* Releases the resources of the event subscriber. */
		~EventSubscriber(void) noexcept
		{
			if (hndlr) delete_s(hndlr);
		}

		/* Copies the data from the specified subscriber to this subscriber. */
		_Check_return_ EventSubscriber<_STy, _ArgTy...>& operator =(_In_ const EventSubscriber<_STy, _ArgTy...> &other)
		{
			if (this != &other)
			{
				if (hndlr) delete_s(hndlr);

				id = other.id;
				hndlr = other.hndlr->Copy();
			}

			return *this;
		}

		/* Moves the data from the specified subscriber to this subscriber. */
		_Check_return_ EventSubscriber<_STy, _ArgTy...>& operator =(_In_ EventSubscriber<_STy, _ArgTy...> &&other)
		{
			if (this != &other)
			{
				if (hndlr) delete_s(hndlr);

				id = other.id;
				hndlr = other.hndlr;

				other.id = 0;
				other.hndlr = nullptr;
			}

			return *this;
		}

		/* Warning is checked and code will work as intended. */
#pragma warning (push)
#pragma warning(disable:4458)
		/* Checks whether the subscriber has the same ID. */
		_Check_return_ inline bool operator ==(_In_ int64 id) const
		{
			return this->id == id;
		}
#pragma warning (pop)

		/* Warning is checked and code will work as intended. */
#pragma warning (push)
#pragma warning(disable:4458)
		/* Checks whether the subscriber has a different ID. */
		_Check_return_ inline bool operator !=(_In_ int64 id) const
		{
			return this->id != id;
		}
#pragma warning (pop)

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