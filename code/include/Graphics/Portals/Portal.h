#pragma once
#include "GameLogic\WorldObject.h"
#include "Graphics\Models\Mesh.h"

namespace Plutonium
{
	/* Defines a portal to another sector. */
	struct Portal
		: public WorldObject
	{
	public:
		/* Whether the portal can be used. */
		bool Enabled;
		/* The destination portal. */
		WorldObject *Destination;

		Portal(_In_ Vector3 position);
		Portal(_In_ const Portal &value) = delete;
		Portal(_In_ Portal &&value) = delete;
		/* Releases the resrouces allocated by the portal. */
		~Portal(void);

		_Check_return_ Portal& operator =(_In_ const Portal &other) = delete;
		_Check_return_ Portal& operator =(_In_ Portal &&other) = delete;

	private:
		friend struct PortalRenderer;

		Mesh *mesh;

		Matrix GetInverseView(const Matrix &view);
		Matrix GetClippedProjection(const Matrix &view, const Matrix &proj);
	};
}