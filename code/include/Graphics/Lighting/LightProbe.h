#pragma once
#include "Content/AssetFetcher.h"
#include "Graphics/Models/Category.h"

namespace Pu
{
	class LightProbeRenderer;
	class LightProbeUniformBlock;
	class Framebuffer;
	class DepthBuffer;

	/* Defines a data contrainer for light probes. */
	class LightProbe
	{
	public:
		/* Defines how often the light probe should be updated. */
		enum class CycleMode
		{
			/* The light probe never updates, instead it used a premade texture. */
			Baked,
			/* The light probe only updates when specifically requested. */
			OnCommand,
			/* The light probe updates on a fixed interval. */
			Interval,
			/* The light probe updates every tick. */
			Tick
		};

		/* Specifies what effects the light probe. */
		ModelCategory Details;

		/* Initializes an empty instance of a light probe. */
		LightProbe(void);
		/* Initializes a new baked instance of a light probe. */
		LightProbe(_In_ AssetFetcher &fetcher, _In_ TextureCube &baked);
		/* Initializes a new updatable instance of a light probe. */
		LightProbe(_In_ LightProbeRenderer &renderer, _In_ Extent2D resolution);
		LightProbe(_In_ const LightProbe&) = delete;
		/* Move constructor. */
		LightProbe(_In_ LightProbe &&value);
		/* Releases the resources allocated by the light probe. */
		~LightProbe(void)
		{
			Destroy();
		}

		_Check_return_ LightProbe& operator =(_In_ const LightProbe&) = delete;
		/* Move assignment. */
		_Check_return_ LightProbe& operator =(_In_ LightProbe &&other);

		/* Sets the position of the light probe. */
		void SetPosition(_In_ Vector3 value);
		/* Sets the range of the light probe frustum. */
		void SetRange(_In_ float near, _In_ float far);
		/* Sets the update cycle mode of the light probe. */
		void SetCycle(_In_ CycleMode mode, _In_opt_ float interval = 0.0f);
		/* Signals any non-baked light probe to update in the next update cycle. */
		void ForceUpdate(void);
		/* Returns whether the light probe should be re-rendered. */
		_Check_return_ bool ShouldUpdate(_In_ float dt);
		/* Gets whether the AABB should be culled when rendering to this light probe. */
		_Check_return_ bool Cull(_In_ const AABB &bb) const;
		/* Gets the viewport that should be used when rendering to this light probe. */
		_Check_return_ Viewport GetViewport(void) const;

		/* Gets the output texture of the light probe. */
		_Check_return_ inline const TextureCube& GetTexture(void) const
		{
			return *texture;
		}

		/* Gets whether the light probe can be used. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return !locked.load();
		}

		/* Gets the position of the light probe in the world. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return position;
		}

		/* Indicates that this light probe is being updated and cannot be used. */
		inline void Lock(void)
		{
			locked.store(true);
		}

		/* Indicates that the light probe is done updating and can be used again. */
		inline void Unlock(void) 
		{
			locked.store(false);
		}

	private:
		friend class LightProbeRenderer;

		Vector3 position;
		float near, far, interval, time;
		Matrix transforms[6];
		CycleMode cycleMode;
		std::atomic_bool locked;
		
		AssetFetcher *fetcher;
		Image *image;
		ImageView *view;
		Sampler *sampler;
		TextureCube *texture;

		LightProbeRenderer *renderer;
		LightProbeUniformBlock *block;
		Framebuffer *framebuffer;
		DepthBuffer *depth;

		void OnRenderpassDone(Renderpass&);
		void CalculateFrustums(void);
		void Destroy(void);
	};
}