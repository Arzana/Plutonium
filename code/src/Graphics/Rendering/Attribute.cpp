#include "Graphics\Rendering\Attribute.h"
#include "Core\Diagnostics\Logging.h"

Plutonium::Attribute::~Attribute(void)
{
	/* Make sure the attribute is disabled on release. */
	if (enabled) Disable();
}

void Plutonium::Attribute::Initialize(bool norm, int32 stride, const void * offset)
{
	/* On debug check if attribute is valid. */
#if defined(DEBUG)
	LOG_THROW_IF(ptr == -1, "Attempting to initialize a invalid attribute!");
#endif

	if (type == FieldType::Double) glVertexAttribLPointer(ptr, initSize, initType, stride, offset);
	else glVertexAttribPointer(ptr, initSize, initType, norm, stride, offset);
}

void Plutonium::Attribute::Enable(void)
{
	/* On debug check if attribute is valid. */
#if defined(DEBUG)
	LOG_THROW_IF(ptr == -1, "Attempting to enable a invalid attribute!");
#endif

	/* Enable the buffer. */
	if (!enabled)
	{
		glEnableVertexAttribArray(ptr);
		enabled = true;
	}
	else LOG_WAR("Attempting to enable enabled attribute %s!", name);
}

void Plutonium::Attribute::Disable(void)
{
	/* On debug check if attribute is valid. */
#if defined(DEBUG)
	LOG_THROW_IF(ptr == -1, "Attempting to disable a invalid attribute!");
#endif

	/* Disable buffer. */
	if (enabled)
	{
		glDisableVertexAttribArray(ptr);
		enabled = false;
	}
	else LOG_WAR("Attempting to disable disabled attribute %s!", name);
}

Plutonium::Attribute::Attribute(void)
	: Field(), enabled(false), initType(-1), initSize(-1)
{}

Plutonium::Attribute::Attribute(int32 ptr, const char * name, uint32 type)
	: Field(ptr, name, type), enabled(false)
{
	switch (this->type)
	{
	case (FieldType::Invalid):
	case (FieldType::Matrix):
	case (FieldType::Texture):
		/* These values cannot (or should not) be passed as attributes. */
		LOG_THROW("Cannot pass %s as a shader attribute!", _CrtGetFieldVisualType(this->type));
		return;
	case (FieldType::Vect2):
		/* Correct the type and size to a 2D float. */
		initSize = 2;
		initType = GL_FLOAT;
		break;
	case (FieldType::Vect3):
		/* Correct the type and size to a 3D float. */
		initSize = 3;
		initType = GL_FLOAT;
		break;
	case (FieldType::Vect4):
		/* Correct the type and size to a 4D float. */
		initSize = 4;
		initType = GL_FLOAT;
		break;
	default:
		/* Size is default and type can be copied over. */
		initSize = 1;
		initType = static_cast<int32>(type);
		break;
	}
}