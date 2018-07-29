#pragma once
#include "Content\AssetLoader.h"
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Mesh.h"

namespace Plutonium
{
	/* Defines a base class for all material types. */
	struct Material
	{
	public:
		/* The name of this material. */
		const char *Name;
		/* Whether this material should be rendered. */
		bool Visible;

#if defined (_DEBUG)
		/* A unique random color assigned to the material to use in debug rendering (only available on debug mode!). */
		Color Debug;
#endif

		/* Initializes a material with default properties. */
		Material(_In_ const char *name, _In_ AssetLoader *loader);
		Material(_In_ const Material &other) = delete;
		Material(_In_ Material &&other) = delete;
		/* Releases the resources allocated with this material. */
		~Material(void);

		_Check_return_ Material& operator =(_In_ const Material &other) = delete;
		_Check_return_ Material& operator =(_In_ Material &&other) = delete;

	protected:
		/* The asset loader associated with this material. */
		AssetLoader *loader;

		/* Attempts to realse the texture either from the asset loader or as a simple delete statement. */
		void ReleaseTexture(_In_ TextureHandler texture);
	};
}