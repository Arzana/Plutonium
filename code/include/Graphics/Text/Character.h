#pragma once
#include "Graphics\Models\Texture.h"

namespace Plutonium
{
	/* Defines all the information needed for a texture bases character. */
	struct Character
	{
		/* The char variant of the texture character in this object. */
		char Key;
		/* The texture associated with this character. */
		Texture *Texture;
		/* The size of the character. */
		Vector2 Size;
		/* The offset from the render position. */
		Vector2 Bearing;
		/* The horizontal space added to the position after this character is drawn. */
		uint32 Advance;

		/* Initializes a new empty instance a character. */
		Character(void)
			: Key('\0'), Texture(nullptr), Size(), Bearing(), Advance(0)
		{}
	};
}