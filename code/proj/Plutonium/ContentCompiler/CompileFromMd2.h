#pragma once
#include "CommandLineArguments.h"
#include "PumData.h"

/* Defines a simple 3 vertex face, defined by vertex indices and texture indices. */
struct md2_triangle_t
{
	/* The indices to the vertices of the triangle. */
	Pu::uint16 vertex_indices[3];
	/* The indices to the textures uv's of the triangle. */
	Pu::uint16 texture_indices[3];
};

/* Defines a simple 3D vertex. */
struct md2_vertex_t
{
	/* The position of the vertex. */
	Pu::Vector3 position;
	/* The normal at the position. */
	Pu::Vector3 normal;
};

/* Defines a animation frame. */
struct md2_frame_t
{
	/* The name of the frame. */
	Pu::string name;
	/* The scale that needs to be applied to all vertices in the frame. */
	Pu::Vector3 scale;
	/* The translation that needs to be applied to all vertices in the frame. */
	Pu::Vector3 translation;
	/* The vertices blob that the frame uses. */
	Pu::vector<md2_vertex_t> vertices;
};

/* Defines the result of an MD2 file load operation. */
struct Md2LoaderResult
{
	/* The required textures for the model (can be none). */
	Pu::vector<Pu::string> textures;
	/* The texture uv's at the model uses. */
	Pu::vector<Pu::Vector2> texcoords;
	/* All the faces that the model defines. */
	Pu::vector<md2_triangle_t> shapes;
	/* All animation frames of the model (can hold multiple animations). */
	Pu::vector<md2_frame_t> frames;
};

/* Excecutes phase one of the MD2 loading and parsing process. */
int LoadMd2(const CLArgs &args, Md2LoaderResult &result);
void Md2ToPum(const CLArgs &args, Md2LoaderResult &input, PumIntermediate &result);