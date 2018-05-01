#pragma once
#include "WorldObject.h"
#include "Game.h"

namespace Plutonium
{
	/* Defines a dynamic world object. */
	struct DynamicObject
		: WorldObject
	{
	public:
		/* Initializes a new instance of a dynamic object. */
		DynamicObject(_In_ Game *game, _In_ const char *model, _In_ const char *texture, _In_ int loadWeight, _In_ Initializer initializer);
		DynamicObject(_In_ const DynamicObject &value) = delete;
		DynamicObject(_In_ DynamicObject &&value) = delete;
		/* Releases the resources allocated by the object. */
		~DynamicObject(void);

		_Check_return_ DynamicObject& operator =(_In_ const DynamicObject &other) = delete;
		_Check_return_ DynamicObject& operator =(_In_ DynamicObject &&other) = delete;

		/* Gets the underlying model of the object. */
		_Check_return_ inline const DynamicModel* GetModel(void) const
		{
			return model;
		}

		/* Plays a specified animation. */
		void PlayAnimation(_In_ const char *name);
		/* Updates the model. */
		void Update(_In_ float dt);

	protected:
		/* Gets the current animations frame mesh. */
		_Check_return_ inline const Mesh* GetCurrentFrame(void) const
		{
			return model->animations.at(curAnim)->Frames.at(curFrame);
		}

		/* Gets the next animations frame mesh. */
		_Check_return_ inline const Mesh* GetNextFrame(void) const
		{
			return model->animations.at(curAnim)->Frames.at(nextFrame);
		}

	private:
		friend struct DynamicRenderer;
		friend struct WireframeRenderer;

		Game *parent;
		DynamicModel *model;
		int percentage;
		Initializer initializer;

		bool running;
		int frameMoveMod;
		float accumTime, mixAmnt;
		size_t curAnim, curFrame, nextFrame;

		void Initialize(void);
		void OnLoadComplete(const AssetLoader*, DynamicModel *result);
		void MoveFrame(void);
	};
}