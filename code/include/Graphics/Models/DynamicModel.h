#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "AnimationInfo.h"
#include "GameLogic\WorldObject.h"

namespace Plutonium
{
	/* Defines a function template for a animation initializer function. */
	using Initializer = void(*)(_In_ const char *Name, _Out_ PlayBackFlags &flags, _Out_ float &fps);

	/* Defines a basic animated model that can be placed in the world. */
	struct DynamicModel
		: public WorldObject
	{
	public:
		DynamicModel(_In_ const DynamicModel &value) = delete;
		DynamicModel(_In_ DynamicModel &&value) = delete;
		/* Releases the resources allocated by the model. */
		~DynamicModel(void);

		_Check_return_ DynamicModel& operator =(_In_ const DynamicModel &other) = delete;
		_Check_return_ DynamicModel& operator =(_In_ DynamicModel &&other) = delete;

		/* Loads a model from a specified .md2 file (requires delete!). */
		_Check_return_ static DynamicModel* FromFile(_In_ const char *path, _In_opt_ const char *texture = nullptr);

		/* Plays a specified animation. */
		void PlayAnimation(_In_ const char *name);
		/* Initializes the animations specified by the model. */
		void Initialize(_In_ Initializer func);
		/* Updates the model. */
		void Update(_In_ float dt);

	protected:
		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

		/* Gets the current animations frame mesh. */
		_Check_return_ inline const Mesh* GetCurrentFrame(void) const
		{
			return animations.at(curAnim)->Frames.at(curFrame);
		}

		/* Gets the next animations frame mesh. */
		_Check_return_ inline const Mesh* GetNextFrame(void) const
		{
			return animations.at(curAnim)->Frames.at(nextFrame);
		}

	private:
		friend struct DynamicRenderer;

		bool running;
		int frameMoveMod;
		float accumTime, mixAmnt;
		size_t curAnim, curFrame, nextFrame;
		std::vector<AnimationInfo*> animations;
		Texture *skin;

		DynamicModel(void);

		void MoveFrame(void);
		void SplitFrames(std::vector<Mesh*> meshes);
	};
}