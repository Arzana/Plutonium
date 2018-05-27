#include "Graphics\Renderer.h"

Plutonium::Renderer::~Renderer(void)
{
	delete_s(shdr);
}

void Plutonium::Renderer::Begin(void)
{
	shdr->Begin();
}

void Plutonium::Renderer::End(void)
{
	shdr->End();
}

void Plutonium::Renderer::DrawTris(const Buffer * buffer, int32 start)
{
	glDrawArrays(GL_TRIANGLES, start, static_cast<GLsizei>(buffer->GetElementCount()));
}

Plutonium::Renderer::Renderer(Shader * shader)
	: shdr(shader)
{}