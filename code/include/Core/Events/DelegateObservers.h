#pragma once
#include <sal.h>

namespace Plutonium
{
	/* provides a interface for a generic delegate. */
	template <typename _STy, typename ... _ArgTy>
	struct DelegateBase
	{
		/* Defines a method for invoking this delegate. */
		virtual void Invoke(_In_ const _STy *sender, _In_ _ArgTy ... args) = 0;
		/* Defines a method for cloning the derived delegate (requires delete!). */
		_Check_return_ virtual DelegateBase<_STy, _ArgTy...>* Copy(void) = 0;
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

		/* Copies this delegate (requires delete!). */
		_Check_return_ virtual DelegateBase<_STy, _ArgTy...>* Copy(void) override
		{
			return new DelegateFunc<_STy, _ArgTy...>(hndlr);
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

		/* Copies this delegate (requires delete!). */
		_Check_return_ virtual DelegateBase<_STy, _ArgTy...>* Copy(void) override
		{
			return new DelegateMethod<_STy, _CTy, _ArgTy...>(obj, hndlr);
		}

	private:
		_CTy *obj;
		HandlerType hndlr;
	};

	/* Provides a structure for a lambda style generic delegate. */
	template <typename _STy, typename _LTy, typename ... _ArgTy>
	struct DelegateLambda
		: public DelegateBase<_STy, _ArgTy...>
	{
	public:
		/* Initializes a new instance of a lambda style generic delegate. */
		DelegateLambda(_In_ const _LTy &lambda)
			: lambda(lambda)
		{}

		/* Invokes this delegate. */
		virtual void Invoke(_In_ const _STy *sender, _In_ _ArgTy ... args) override
		{
			return lambda.operator()(sender, args...);
		}

		/* Copies this delegate (requires delete!). */
		_Check_return_ virtual DelegateBase<_STy, _ArgTy...>* Copy(void) override
		{
			return new DelegateLambda<_STy, _LTy, _ArgTy...>(lambda);
		}

	private:
		_LTy lambda;
	};
}