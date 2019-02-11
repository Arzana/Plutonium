#include "Graphics/Resources/BufferAccessor.h"

Pu::BufferAccessor::BufferAccessor(BufferView & view, size_t offset)
	: view(view), offset(offset), elementCount(0), elementType(FieldTypes::Invalid)
{}

Pu::BufferAccessor::BufferAccessor(const BufferAccessor & value)
	: view(value.view), offset(value.offset), elementCount(value.elementCount), elementType(value.elementType)
{}

Pu::BufferAccessor::BufferAccessor(BufferAccessor && value)
	: view(value.view), offset(value.offset), elementCount(value.elementCount), elementType(value.elementType)
{}

Pu::BufferAccessor & Pu::BufferAccessor::operator=(BufferAccessor && other)
{
	if (this != &other)
	{
		view = std::move(other.view);
		offset = other.offset;
		elementCount = other.elementCount;
		elementType = other.elementType;
	}

	return *this;
}

void Pu::BufferAccessor::SetData(FieldTypes type, const void * data, size_t count)
{
	elementCount = count;
	elementType = type;

	view.SetData(data, offset, sizeof_fieldType(type), count);
}