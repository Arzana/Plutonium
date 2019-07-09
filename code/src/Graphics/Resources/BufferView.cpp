#include "Graphics/Resources/BufferView.h"

Pu::BufferView::BufferView(const Buffer & buffer, size_t stride)
	: buffer(&buffer), offset(0), size(buffer.size), stride(stride)
{}

Pu::BufferView::BufferView(const Buffer & buffer, size_t offset, size_t size, size_t stride)
	: buffer(&buffer), offset(offset), size(size), stride(stride)
{}

Pu::BufferView::BufferView(const BufferView & value)
	: buffer(value.buffer), offset(value.offset), size(value.size), stride(value.stride)
{}

Pu::BufferView::BufferView(BufferView && value)
	: buffer(value.buffer), offset(value.offset), size(value.size), stride(value.stride)
{}

Pu::BufferView & Pu::BufferView::operator=(BufferView && other)
{
	if (this != &other)
	{
		buffer = other.buffer;
		offset = other.offset;
		size = other.size;
		stride = other.stride;
	}

	return *this;
}