#pragma once
#include <sal.h>

namespace Plutonium
{
	/* provides a interface for a generic delegate. */
	template <typename _STy, typename ... _ArgTy>
	struct DelegateBase
	{
		/* Defines a virtual destructor so the destructors of child types will always be called upon finalization. */
		virtual ~DelegateBase(void) noexcept
		{}

		/* Defines a method for invoking this delegate. */
		virtual void Invoke(_In_ const _STy *sender, _In_ _ArgTy ... args) = 0;
	};

	/* Provides a structure for a function style generic delegate. */
	template <typename _STy, typename ... _ArgTy>
	struct DelegateFunc
		: public DelegateBase<_STy, _ArgTy...>
	{
	public:
		/* Defines the type of handler this delegate can store. */
		using HandlerType = void(*)(_In_ const _STy *sender, _In_ _ArgTy ... args);

		/* Initializes a new instance of a function style generic delegate. */
		DelegateFunc(_In_ HandlerType func)
			: DelegateBase(), hndlr(func)
		{}

		/* Invokes this delegate. */
		virtual void Invoke(_In_ const _STy *sender, _In_ _ArgTy ... args) override
		{
			hndlr(sender, args...);
		}

	private:
		HandlerType hndlr;
	};

	/* provides a structure for a method style generic delegate. */
	template <typename _STy, typename _CTy, typename ... _ArgTy>
	struct DelegateMethod
		: public DelegateBase<_STy, _ArgTy...>
	{
	public:
		/* Defines the type of handler this delegate can store. */
		using HandlerType = void(_CTy::*)(_In_ const _STy *sender, _In_ _ArgTy ... args);

		/* Initializes a new instance of a method style generic delegate. */
		DelegateMethod(_In_ _CTy *cnt, HandlerType func)
			: DelegateBase(), obj(cnt), hndlr(func)
		{}

		/* Invokes this delegate. */
		virtual void Invoke(_In_ const _STy *sender, _In_ _ArgTy ... args) override
		{
			(obj->*hndlr)(sender, args...);
		}

	private:
		_CTy *obj;
		HandlerType hndlr;
	};
}