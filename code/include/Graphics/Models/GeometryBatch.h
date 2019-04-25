#pragma once
#include "Graphics/Resources/Mesh.h"
#include "Graphics/Materials/Material.h"

namespace Pu
{
	/* Combines a mesh with a specific material. */
	class GeometryBatch
	{
	public:
		/* Defines the geometry using in the render batch. */
		const Mesh &Geometry;
		/* Defines the material that should be used during the render batch. */
		const Material &Material;

		/* Initializes a new instance of a geometry batch. */
		GeometryBatch(_In_ const Mesh &mesh, _In_ const Pu::Material &material)
			: Geometry(mesh), Material(material)
		{}
	};
}