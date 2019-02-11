#include "Graphics/Resources/BufferView.h"

Pu::BufferView::BufferView(Buffer & buffer, size_t offset, size_t size, size_t stride)
	: buffer(buffer), offset(offset), size(size), stride(stride)
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
		buffer = std::move(other.buffer);
		offset = other.offset;
		size = other.size;
		stride = other.stride;
	}

	return *this;
}

/* size/offset hides class member, checked and works as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::BufferView::SetData(const void * data, size_t offset, size_t elementSize, size_t elementCount)
{
	/* Stride is either the view stride (if set) or just the element stride. */
	buffer.SetData(data, elementSize * elementCount, elementSize, this->offset + offset, stride ? stride : elementSize);
}
#pragma warning(pop)