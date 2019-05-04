#pragma once
#include "Core/Math/Vector4.h"

namespace Pu
{
	/* Defines a vertex layout that stores a position, normal, texture coordinate, joint indeces and joint weights. */
	struct SkinnedAnimated
	{
	public:
		/* The location of the vertex. */
		Vector3 Position;
		/* The surface normal of the vertex. */
		Vector3 Normal;
		/* The texture coordinate of the vertex. */
		Vector2 TexCoord;
		/* The indeces of the joints affecting the vertex. */
		//uint16 Joints[4];
		/* The weights of each joints. */
		//Vector4 Weights;
	};
}