#pragma once
#include "Field.h"

namespace Plutonium
{
	/* Defines a helper object for shader attributes. */
	struct Attribute
		: public Field
	{
	public:
		Attribute(_In_ const Attribute &value) = delete;
		Attribute(_In_ Attribute &&value) = delete;
		/* Releases the resources allocated by the shader attribute. */
		~Attribute(void);

		_Check_return_ Attribute& operator =(_In_ const Attribute &other) = delete;
		_Check_return_ Attribute& operator = (_In_ Attribute &&other) = delete;

		/* 
		Initializes the attribute buffer, providing buffer information to OpenGL. 
		Normalized is not using for GL_DOUBLE values.
		*/
		void Initialize(_In_ bool normalized, _In_ int32 stride, _In_ const void *offset);
		/* Enables the attribute. */
		void Enable(void);
		/* Disables the attribute. */
		void Disable(void);

	protected:
		/* Returns whether this field is a attribute or a uniform. */
		_Check_return_ inline virtual bool IsAttribute(void) const override
		{
			return true;
		}

	private:
		friend struct Shader;

		bool initialized, enabled;

		Attribute(void);
		Attribute(int32 ptr, const char *name, uint32 type);
	};
}