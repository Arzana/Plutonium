#include "Graphics/Resources/BufferAccessor.h"

Pu::BufferAccessor::BufferAccessor(BufferView & view, FieldTypes type, size_t offset)
	: view(view), elementType(type), offset(offset)
{}

Pu::BufferAccessor::BufferAccessor(const BufferAccessor & value)
	: view(value.view), elementType(value.elementType), offset(value.offset)
{}

Pu::BufferAccessor::BufferAccessor(BufferAccessor && value)
	: view(value.view), elementType(value.elementType), offset(value.offset)
{}

Pu::BufferAccessor & Pu::BufferAccessor::operator=(BufferAccessor && other)
{
	if (this != &other)
	{
		view = std::move(other.view);
		elementType = other.elementType;
		offset = other.offset;
	}

	return *this;
}

void Pu::BufferAccessor::SetData(const void * data, size_t count)
{
	if (count > GetElementCount()) Log::Fatal("Cannot set data on accessor (buffer too small)!");
	view.SetData(data, offset, sizeof_fieldType(elementType), count);
}