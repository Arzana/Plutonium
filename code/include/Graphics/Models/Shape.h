#pragma once
#include "Mesh.h"
#include "Texture.h"

namespace Plutonium
{
	/* Defines the internal layout of the model. */
	struct Shape
	{
		/* The vertices of the model. */
		Mesh *Mesh;
		/* The material properties of the model. */
		Texture *Material;

		/* Initializes an empty instance of the shape object. */
		Shape(void)
			: Mesh(nullptr), Material(nullptr)
		{}

		/* Initializes a new instance of a shape. */
		Shape(_In_ Plutonium::Mesh *mesh, _In_ Texture *mat)
			: Mesh(mesh), Material(mat)
		{}
	};
}