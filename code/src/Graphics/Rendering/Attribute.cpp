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
	LOG_WAR_IF(initialized, "Re-initializing attribute %s, is this intended?", name);
#endif

	/* Convert the frameworks types to OpenGL types and correct the size for the vector types. */
	int32 openGLType;
	int32 openGlSize;
	switch (type)
	{
		case (FieldType::Invalid):
		case (FieldType::Matrix):
		case (FieldType::Texture):
			/* These values cannot (or should not) be passed as attributes. */
			LOG_THROW("Cannot pass %s as a shader attribute!", _CrtGetFieldVisualType(type));
			return;
		case (FieldType::Double):
			/* Double needs to be passed using the long pointer function. */
			glVertexAttribLPointer(ptr, 1, GL_DOUBLE, stride, offset);
			initialized = true;
			return;
		case (FieldType::Vect2):
			/* Correct the type and size to a 2D float. */
			openGlSize = 2;
			openGLType = GL_FLOAT;
			break;
		case (FieldType::Vect3):
			/* Correct the type and size to a 3D float. */
			openGlSize = 3;
			openGLType = GL_FLOAT;
			break;
		case (FieldType::Vect4):
			/* Correct the type and size to a 4D float. */
			openGlSize = 4;
			openGLType = GL_FLOAT;
			break;
		default:
			/* Size is default and type can be copied over. */
			openGlSize = 1;
			openGLType = static_cast<int32>(type);
			break;
	}

	/* Provide info to OpenGL. */
	glVertexAttribPointer(ptr, openGlSize, openGLType, norm, stride, offset);
	initialized = true;
}

void Plutonium::Attribute::Enable(void)
{
	/* On debug check if attribute is valid. */
#if defined(DEBUG)
	LOG_THROW_IF(ptr == -1, "Attempting to enable a invalid attribute!");
	LOG_THROW_IF(!initialized, "Attempting to enable not-initialized attribute %s!", name);
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
	LOG_THROW_IF(!initialized, "Attempting to disable non-initialized attribute %s!", name);
#endif

	/* Disable buffer. */
	if (enabled)
	{
		glDisableVertexAttribArray(ptr);
		enabled = false;
	}
	else LOG_WAR("Attempting to disable disabled sttribute %s!", name);
}

Plutonium::Attribute::Attribute(void)
	: Field(), initialized(false), enabled(false)
{}

Plutonium::Attribute::Attribute(int32 ptr, const char * name, uint32 type)
	: Field(ptr, name, type), initialized(false), enabled(false)
{}