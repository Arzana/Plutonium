#pragma once
#include "Graphics\Texture.h"
#include "Core\Math\Matrix.h"
#include "Graphics\Color.h"
#include "Field.h"

namespace Plutonium
{
	/* Defines a helper object for shader uniforms. */
	struct Uniform
		: public Field
	{
	public:
		Uniform(_In_ const Uniform &value) = delete;
		Uniform(_In_ Uniform &&value) = delete;

		_Check_return_ Uniform& operator =(_In_ const Uniform &other) = delete;
		_Check_return_ Uniform& operator = (_In_ Uniform &&other) = delete;

		/* Attempts to set the uniform to a specified boolean value. */
		void Set(_In_ bool value);
		/* Attempts to set the uniform to a specified signed integer value. */
		void Set(_In_ int32 value);
		/* Attempts to set the uniform to a specified unsigned integer value. */
		void Set(_In_ uint32 value);
		/* Attempts to set the uniform to a specified single value. */
		void Set(_In_ float value);
		/* Attempts to set the uniform to a specified double value. */
		void Set(_In_ double value);
		/* Attempts to set the uniform to a specified 2D vector value. */
		void Set(_In_ Vector2 value);
		/* Attempts to set the uniform to a specified 3D vector value. */
		void Set(_In_ Vector3 value);
		/* Attempts to set the uniform to a specified 4D vector value. */
		void Set(_In_ Vector4 value);
		/* Attempts to set the uniform to a specified matrix value. */
		void Set(_In_ const Matrix &value);
		/* Attempts to set the uniform to a specified color value. */
		inline void Set(_In_ Color value)
		{
			Set(value.ToVector4());
		}
		/* Attempts to set the uniform to a specified texture value. */
		void Set(_In_ const Texture *value);

	protected:
		/* Returns whether this field is a attribute or a uniform. */
		_Check_return_ inline virtual bool IsAttribute(void) const override
		{
			return false;
		}

	private:
		friend struct Shader;

		int32 sampler;

		Uniform(void);
		Uniform(uint32 ptr, const char *name, int32 type, int32 sampler);
	};
}