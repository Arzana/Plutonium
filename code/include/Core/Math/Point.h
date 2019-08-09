#pragma once
#include "Constants.h"

namespace Pu
{
	/* Defines a two dimensional point. */
	template <typename component_t>
	struct GenPoint
	{
	public:
		/* The X-coordinate of this point. */
		component_t X;
		/* The Y-coordinate of this point. */
		component_t Y;

		/* Initializes an empty instance of a 2D point. */
		GenPoint(void)
			: X(0), Y(0)
		{}

		/* Initializes a new instance of a 2D point with both coordinates set to the specified value. */
		GenPoint(_In_ component_t v)
			: X(v), Y(v)
		{}

		/* Initializes a new instance of a 2D point with both coordinates specified. */
		GenPoint(_In_ component_t x, _In_ component_t y)
			: X(x), Y(y)
		{}

		/* Converts statically between two point types. */
		template <typename other_component_t>
		_Check_return_ inline explicit operator GenPoint<other_component_t>(void) const
		{
			return GenPoint<other_component_t>(static_cast<other_component_t>(X), static_cast<other_component_t>(Y));
		}

		/* Adds two points together. */
		template <typename other_component_t>
		_Check_return_ inline auto operator +(_In_ const GenPoint<other_component_t> &other) const
		{
			return GenPoint<decltype(X + other.X)>(X + other.X, Y + other.Y);
		}

		/* Adds the second point to this point. */
		template <typename other_component_t>
		_Check_return_ inline GenPoint<component_t> operator +=(_In_ const GenPoint<other_component_t> &other)
		{
			X += static_cast<component_t>(other.X);
			Y += static_cast<component_t>(other.Y);
			return *this;
		}
	};

	/* Defines a two dimensional integer point. */
	using Point = GenPoint<int32>;
	/* Defines a two dimensional unsigned integer point. */
	using Size = GenPoint<uint32>;
	/* Defines a two dimensional long integer point. */
	using LPoint = GenPoint<int64>;
	/* Defines a two dimensional unsigned long integer point. */
	using LSize = GenPoint<uint64>;
}