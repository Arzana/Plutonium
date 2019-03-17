#pragma once
#include "GLTFLoader.h"
#include "Graphics/Resources/Mesh.h"
#include "Graphics/Resources/StagingBuffer.h"

namespace Pu
{
	/*
	Most of the binary data in a GLTF .bin file is good to use right away but the meshes aren't in an optimal format.
	All of the animation data and the indeces are good to use, but the mesh data isn't.
	To use it right away we would have to create a buffer view per element in the vertex, which is annoying for writing shaders.
	So we rearrange only the mesh data so it fits our purposes better and just leave the other data as is.
	This results in different buffer views and accossors, so we just create the meshes with this function.
	See GLTFMonsterBufferLayout to see an example of the default layout.
	The meshes will be created by this function but the staging buffers need to be supplied as they rely on a specific logical device.

	TODO: Add animations to this function.
	*/
	void _CrtLoadAndParseGLTF(_In_ const GLTFFile &file, _In_ const vector<std::reference_wrapper<Buffer>> &buffers, _Out_ vector<StagingBuffer*> &stagingBuffers, _Out_ vector<Mesh*> &meshes);
}