#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* provides a interface for a generic delegate. */
	template <typename sender_t, typename ... argument_t>
	class DelegateBase
	{
	public:
		/* Defines the type of this delegate. */
		using base_t = DelegateBase<sender_t, argument_t...>;

		/* Copy constructor. */
		DelegateBase(_In_ const base_t &value)
			: id(value.id)
		{}

		/* Move constructor. */
		DelegateBase(_In_ base_t &&value)
			: id(value.id)
		{}

		/* Copy assignment. */
		_Check_return_ inline base_t& operator =(_In_ const base_t &other)
		{
			id = other.id;
			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline base_t& operator =(_In_ base_t &&other)
		{
			id = other.id;
			return *this;
		}

		/* Checks whether two delegates are equal. */
		_Check_return_ inline bool operator ==(_In_ const int64 &other) const
		{
			return other == id;
		}

		/* Checks whether two delegates differ. */
		_Check_return_ inline bool operator !=(_In_ const int64 &other) const
		{
			return other != id;
		}

		/* Gets the unique indentifier of this delegate. */
		_Check_return_ inline int64 GetID(void) const
		{
			return id;
		}

		/* Defines a method for invoking this delegate. */
		virtual void Invoke(_In_ sender_t &sender, _In_ argument_t ... arg) = 0;
		/* Defines a method for cloning the derived delegate. */
		_Check_return_ virtual base_t* Copy(void) = 0;

	protected:
		/* Initializes a new instance of a delegate base. */
		DelegateBase(_In_ int64 id)
			: id(id)
		{}

		/* Computes a raw delegate hash from the two input parameters. */
		static inline int64 ComputeRawHash(const void *arg1, const void *arg2)
		{
			const int64 first = static_cast<int64>(reinterpret_cast<intptr_t>(arg1));
			const int64 second = static_cast<int64>(reinterpret_cast<intptr_t>(arg2));
			return (first << 32) | second;
		}

	private:
		int64 id;
	};

	/* Provides a structure for a function style generic delegate. */
	template <typename sender_t, typename ... argument_t>
	class DelegateFunc
		: public DelegateBase<sender_t, argument_t...>
	{
	public:
		/* Defines the base type if this function delegate. */
		using base_t = typename DelegateBase<sender_t, argument_t...>::base_t;
		/* Defines the type of this function delegate. */
		using func_t = DelegateFunc<sender_t, argument_t...>;
		/* Defines the type of handler this delegate can store. */
		using handler_t = void(*)(_In_ sender_t &sender, _In_ argument_t ... arg);

		/* Initializes a new instance of a function style generic delegate. */
		DelegateFunc(_In_ handler_t func)
			: base_t(ComputeHash(func)), hndlr(func)
		{}

		/* Copy constructor. */
		DelegateFunc(_In_ const func_t &value)
			: base_t(value), hndlr(value.hndlr)
		{}

		/* Move constructor. */
		DelegateFunc(_In_ func_t &&value)
			: base_t(std::move(value)), hndlr(value.hndlr)
		{}

		/* Copy assignment. */
		_Check_return_ inline func_t& operator =(_In_ const func_t &other)
		{
			base_t::operator=(other);
			hndlr = other.hndlr;
			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline func_t& operator =(_In_ func_t &&other)
		{
			base_t::operator=(std::move(other));
			hndlr = other.hndlr;
			return *this;
		}

		/* Invokes this delegate. */
		virtual inline void Invoke(_In_ sender_t &sender, _In_ argument_t ... arg) override
		{
			hndlr(sender, arg...);
		}

		/* Copies this delegate (requires delete!). */
		_Check_return_ virtual inline base_t* Copy(void) override
		{
			return new func_t(hndlr);
		}

		/* Computes the hash for a function delegate. */
		_Check_return_ static inline int64 ComputeHash(_In_ handler_t hndlr)
		{
			return base_t::ComputeRawHash(nullptr, reinterpret_cast<const void*>(hndlr));
		}

	private:
		handler_t hndlr;
	};

	/* provides a structure for a method style generic delegate. */
	template <typename sender_t, typename container_t, typename ... argument_t>
	class DelegateMethod
		: public DelegateBase<sender_t, argument_t...>
	{
	public:
		/* Defines the base type if this function delegate. */
		using base_t = typename DelegateBase<sender_t, argument_t...>::base_t;
		/* Defines the type of this method delegate. */
		using method_t = DelegateMethod<sender_t, container_t, argument_t...>;
		/* Defines the type of handler this delegate can store. */
		using handler_t = void(container_t::*)(_In_ sender_t &sender, _In_ argument_t ... arg);

		/* Initializes a new instance of a method style generic delegate. */
		DelegateMethod(_In_ container_t &cnt, handler_t func)
			: base_t(ComputeHash(cnt, func)), obj(&cnt), hndlr(func)
		{}

		/* Copy constructor. */
		DelegateMethod(_In_ const method_t &value)
			: base_t(value), obj(value.obj), hndlr(value.hndlr)
		{}

		/* Move constructor. */
		DelegateMethod(_In_ method_t &&value)
			: base_t(std::move(value)), obj(value.obj), hndlr(value.hndlr)
		{}

		/* Copy assignment. */
		_Check_return_ inline method_t& operator =(_In_ const method_t &other)
		{
			if (this != &other)
			{
				base_t::operator=(other);
				obj = other.obj;
				hndlr = other.hndlr;
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline method_t& operator =(_In_ method_t &&other)
		{
			if (this != &other)
			{
				base_t::operator=(std::move(other));
				obj = other.obj;
				hndlr = other.hndlr;
			}

			return *this;
		}

		/* Invokes this delegate. */
		virtual inline void Invoke(_In_ sender_t &sender, _In_ argument_t ... arg) override
		{
			((*obj).*hndlr)(sender, arg...);
		}

		/* Copies this delegate (requires delete!). */
		_Check_return_ virtual inline base_t* Copy(void) override
		{
			return new method_t(*obj, hndlr);
		}

		/* Computes the hash for a method delegate. */
		_Check_return_ static int64 ComputeHash(_In_ const container_t &cnt, _In_ handler_t func)
		{
			return base_t::ComputeRawHash(&cnt, (const void*&)func);
		}

	private:
		container_t *obj;
		handler_t hndlr;
	};

	/* Provides a structure for a lambda style generic delegate. */
	template <typename sender_t, typename lambda_t, typename ... argument_t>
	class DelegateLambda
		: public DelegateBase<sender_t, argument_t...>
	{
	public:
		/* Defines the base type if this function delegate. */
		using base_t = typename DelegateBase<sender_t, argument_t...>::base_t;
		/* Defines the type of this lambda delegate. */
		using func_t = DelegateLambda<sender_t, lambda_t, argument_t...>;
		/* Defines the type of handler this delegate can store. */
		using handler_t = void(lambda_t::*)(_In_ sender_t &sender, _In_ argument_t ... arg);

		/* Initializes a new instance of a lambda style generic delegate. */
		DelegateLambda(_In_ const lambda_t &lambda)
			: base_t(ComputeHash()), lambda(lambda)
		{}

		/* Copy constructor. */
		DelegateLambda(_In_ const func_t &value)
			: base_t(value), lambda(value.lambda)
		{}

		/* Move constructor. */
		DelegateLambda(_In_ func_t &&value)
			: base_t(std::move(value)), lambda(value.lambda)
		{}

		/* Copy assignment. */
		_Check_return_ inline func_t& operator =(_In_ const func_t &other)
		{
			base_t::operator=(other);
			lambda = other.lambda;
			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline func_t& operator =(_In_ func_t &&other)
		{
			base_t::operator=(std::move(other));
			lambda = other.lambda;
			return *this;
		}

		/* Invokes this delegate. */
		virtual inline void Invoke(_In_ sender_t &sender, _In_ argument_t ... arg) override
		{
			return lambda.operator()(sender, arg...);
		}

		/* Copies this delegate (requires delete!). */
		_Check_return_ virtual inline base_t* Copy(void) override
		{
			return new func_t(lambda);
		}

		/* COmputes the hash for a lambda delegate. */
		_Check_return_ static inline int64 ComputeHash(void)
		{
			handler_t hndlr = reinterpret_cast<handler_t>(&lambda_t::operator());
			return base_t::ComputeRawHash(nullptr, (const void*&)hndlr);
		}

	private:
		lambda_t lambda;
	};
}