#pragma once
#include "GameLogic\WorldObject.h"
#include "Graphics\Mesh.h"

namespace Plutonium
{
	struct EuclidRoom;

	/* Defines a portal to another sector. */
	struct Portal
		: public WorldObject
	{
	public:
		/* Whether the portal can be used. */
		bool Enabled;
		/* The destination portal. */
		EuclidRoom *Destination;

		Portal(_In_ const Portal &value) = delete;
		Portal(_In_ Portal &&value) = delete;
		/* Releases the resrouces allocated by the portal. */
		~Portal(void);

		_Check_return_ Portal& operator =(_In_ const Portal &other) = delete;
		_Check_return_ Portal& operator =(_In_ Portal &&other) = delete;

	private:
		friend struct PortalRenderer;
		friend struct EuclidRoom;

		Portal(Mesh *mesh);

		Mesh *mesh;
		Vector3 center;

		Matrix GetInverseView(const Matrix &view);
		Matrix GetClippedProjection(const Matrix &view, const Matrix &proj);
	};
}