#pragma once
#include "Graphics\Shape.h"
#include "Core\Collections\Tree.h"
#include "Graphics\Portals\Portal.h"
#include "Graphics\Portals\SceneRenderArgs.h"

namespace Plutonium
{
	/* Defines a room that can be rendered using portals. */
	struct EuclidRoom
		: public WorldObject
	{
	public:
		EuclidRoom(_In_ const EuclidRoom &value) = delete;
		EuclidRoom(_In_ EuclidRoom &&value) = delete;
		/* Releases the resources allocated by the room. */
		~EuclidRoom(void);

		_Check_return_ EuclidRoom& operator =(_In_ const EuclidRoom &other) = delete;
		_Check_return_ EuclidRoom& operator =(_In_ EuclidRoom &&other) = delete;

		/* Loads a model from a specified .obj file (requires delete!). */
		_Check_return_ static std::vector<EuclidRoom*> FromFile(_In_ const char *path);

		/* Adds the portal argument from this room to the render list. */
		void AddPortals(_In_ Tree<PortalRenderArgs> *portals) const;

		/* Gets the active gravitational force of the room. */
		_Check_return_ inline Vector3 GetRoomGravity(void) const
		{
			return gravityForce;
		}

		/* Gets the indentifier of this room. */
		_Check_return_ inline int32 GetID(void) const
		{
			return id;
		}

		/* Set the scale of the room. */
		virtual void SetScale(_In_ float scale) override;

	protected:
		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend struct StaticRenderer;

		EuclidRoom(void)
			: WorldObject()
		{}

		int32 id;
		Vector3 gravityForce;
		std::vector<Portal*> portals;
		std::vector<Shape*> shapes;
	};
}