#pragma once
#include <assert.h>
#include "Core/Collections/vector.h"
#include "Physics/Objects/PhysicsHandle.h"
#include "Physics/Properties/PhysicalProperties.h"

namespace Pu
{
	/* Defines an object used to query and store material properties. */
	class MaterialDatabase
	{
	public:
		/* Initializes a new instance of a material database. */
		MaterialDatabase(void) = default;
		MaterialDatabase(_In_ const MaterialDatabase&) = delete;
		/* Move constructor. */
		MaterialDatabase(_In_ MaterialDatabase &&value) = default;

		_Check_return_ MaterialDatabase& operator =(_In_ const MaterialDatabase&) = delete;
		/* Move assignment. */
		_Check_return_ MaterialDatabase& operator =(_In_ MaterialDatabase &&other) = default;

		/* Gets the material with the specified id. */
		_Check_return_ inline const PhysicalProperties& operator [](_In_ PhysicsHandle id) const
		{
			assert(physics_get_type(id) == PhysicsType::Material);
			return materials[physics_get_lookup_id(id)];
		}

		/* Adds a new material to the database and returns its identifier */
		inline PhysicsHandle Add(_In_ const PhysicalProperties &properties)
		{
			materials.emplace_back(properties);
			return create_physics_handle(PhysicsType::Material, materials.size() - 1);
		}

	private:
		vector<PhysicalProperties> materials;
	};
}