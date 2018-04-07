#pragma once
#include "Graphics\Texture.h"

namespace Plutonium
{
	/* Defines all the information needed for a texture bases character. */
	struct Character
	{
		/* The char variant of the texture character in this object. */
		int32 Key;
		/* The texture bounds of the character. */
		Rectangle Bounds;
		/* The size of the character. */
		Vector2 Size;
		/* The offset from the render position. */
		Vector2 Bearing;
		/* The horizontal space added to the position after this character is drawn. */
		uint32 Advance;

		/* Initializes a new empty instance a character. */
		Character(void)
			: Key('\0'), Bounds(), Size(), Bearing(), Advance(0)
		{}

		Character(_In_ const Character &value) = delete;
		Character(_In_ Character &&value) = delete;

		_Check_return_ Character& operator =(_In_ const Character &other) = delete;
		_Check_return_ Character& operator =(_In_ Character &&other) = delete;
	};
}