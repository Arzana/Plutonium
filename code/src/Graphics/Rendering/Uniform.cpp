#include "Graphics\Rendering\Uniform.h"
#include "Core\Diagnostics\Logging.h"

/* make sure we only check if the correct type is passed on debug mode. */
#if defined(DEBUG)
#define DBG_CHECK(type)		CheckInput(type)
#else
#define DBG_CHECK(...)
#endif

Plutonium::Uniform::Uniform(void)
	: Field(), sampler(-1)
{}

Plutonium::Uniform::Uniform(uint32 ptr, const char * name, int32 type, int32 sampler)
	: Field(ptr, name, type), sampler(sampler)
{}

void Plutonium::Uniform::Set(bool value)
{
	DBG_CHECK(FieldType::Bool);
	glUniform1i(ptr, value);
}

void Plutonium::Uniform::Set(int32 value)
{
	DBG_CHECK(FieldType::Int);
	glUniform1i(ptr, value);
}

void Plutonium::Uniform::Set(uint32 value)
{
	DBG_CHECK(FieldType::UInt);
	glUniform1ui(ptr, value);
}

void Plutonium::Uniform::Set(float value)
{
	DBG_CHECK(FieldType::Float);
	glUniform1f(ptr, value);
}

void Plutonium::Uniform::Set(double value)
{
	DBG_CHECK(FieldType::Double);
	glUniform1d(ptr, value);
}

void Plutonium::Uniform::Set(Vector2 value)
{
	DBG_CHECK(FieldType::Vect2);
	glUniform2f(ptr, value.X, value.Y);
}

void Plutonium::Uniform::Set(Vector3 value)
{
	DBG_CHECK(FieldType::Vect3);
	glUniform3f(ptr, value.X, value.Y, value.Z);
}

void Plutonium::Uniform::Set(Vector4 value)
{
	DBG_CHECK(FieldType::Vect4);
	glUniform4f(ptr, value.X, value.Y, value.Z, value.W);
}

void Plutonium::Uniform::Set(const Matrix & value)
{
	DBG_CHECK(FieldType::Matrix);
	glUniformMatrix4fv(ptr, 1, false, value.GetComponents());
}

void Plutonium::Uniform::Set(const Texture & value)
{
	DBG_CHECK(FieldType::Texture);
	glActiveTexture(GL_TEXTURE0 + sampler);
	glBindTexture(GL_TEXTURE_2D, value.ptr);
	glUniform1i(ptr, sampler);
}