#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "Core\StringFunctions.h"

namespace Plutonium
{
	/* Defines the internal layout of the model. */
	struct Shape
	{
		/* The name of the used material. */
		const char *MaterialName;
		/* The vertices of the model. */
		Mesh *Mesh;
		/* The material properties of the model. */
		Texture *Material;

		/* Initializes an empty instance of the shape object. */
		Shape(void)
			: MaterialName(nullptr), Mesh(nullptr), Material(nullptr)
		{}

		/* Initializes a new instance of a shape. */
		Shape(_In_ const char *name, _In_ Plutonium::Mesh *mesh, _In_ Texture *mat)
			: MaterialName(heapstr(name)), Mesh(mesh), Material(mat)
		{}
	};
}