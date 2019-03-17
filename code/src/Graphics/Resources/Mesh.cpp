#include "Graphics/Resources/Mesh.h"

Pu::Mesh::Mesh(Buffer & buffer, size_t offset, size_t size, size_t stride, PrimitiveTopology topology)
	: BufferView(buffer, offset, size, stride), renderMode(topology),
	idx(nullptr), pos(nullptr), norm(nullptr), tang(nullptr),
	tex1(nullptr), tex2(nullptr), clr(nullptr), jnts(nullptr), wghs(nullptr)
{}

Pu::Mesh::Mesh(const Mesh & value)
	: BufferView(value), renderMode(value.renderMode)
{
	idx = value.idx ? new BufferAccessor(*value.idx) : nullptr;
	pos = value.idx ? new BufferAccessor(*value.pos) : nullptr;
	norm = value.idx ? new BufferAccessor(*value.norm) : nullptr;
	tang = value.idx ? new BufferAccessor(*value.tang) : nullptr;
	tex1 = value.idx ? new BufferAccessor(*value.tex1) : nullptr;
	tex2 = value.idx ? new BufferAccessor(*value.tex2) : nullptr;
	clr = value.idx ? new BufferAccessor(*value.clr) : nullptr;
	jnts = value.idx ? new BufferAccessor(*value.jnts) : nullptr;
	wghs = value.idx ? new BufferAccessor(*value.wghs) : nullptr;
}

Pu::Mesh::Mesh(Mesh && value)
	: BufferView(std::move(value)), renderMode(value.renderMode),
	idx(value.idx), pos(value.pos), norm(value.norm),
	tang(value.tang), tex1(value.tex1), tex2(value.tex2),
	clr(value.clr), jnts(value.jnts), wghs(value.wghs)
{
	value.idx = nullptr;
	value.pos = nullptr;
	value.norm = nullptr;
	value.tang = nullptr;
	value.tex1 = nullptr;
	value.tex2 = nullptr;
	value.clr = nullptr;
	value.jnts = nullptr;
	value.wghs = nullptr;
}

Pu::Mesh & Pu::Mesh::operator=(Mesh && other)
{
	if (this != &other)
	{
		Destroy();

		BufferView::operator=(std::move(other));
		renderMode = other.renderMode;

		idx = other.idx;
		pos = other.pos;
		norm = other.norm;
		tang = other.tang;
		tex1 = other.tex1;
		tex2 = other.tex2;
		clr = other.clr;
		jnts = other.jnts;
		wghs = other.wghs;

		other.idx = nullptr;
		other.pos = nullptr;
		other.norm = nullptr;
		other.tang = nullptr;
		other.tex1 = nullptr;
		other.tex2 = nullptr;
		other.clr = nullptr;
		other.jnts = nullptr;
		other.wghs = nullptr;
	}

	return *this;
}

void Pu::Mesh::Destroy(void)
{
	if (idx) delete idx;
	if (pos) delete pos;
	if (norm) delete norm;
	if (tang) delete tang;
	if (tex1) delete tex1;
	if (tex2) delete tex2;
	if (clr) delete clr;
	if (jnts) delete jnts;
	if (wghs) delete wghs;
}