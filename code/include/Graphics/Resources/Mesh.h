#pragma once
#include "BufferAccessor.h"

namespace Pu
{
	/* Defines a view to vertex data in a model. */
	class Mesh
		: public BufferView
	{
	public:
		/* Copy constructor. */
		Mesh(_In_ const Mesh &value);
		/* Move constructor. */
		Mesh(_In_ Mesh &&value);
		/* Releases the resources allocated by the mesh. */
		~Mesh(void)
		{
			Destroy();
		}

		_Check_return_ Mesh& operator =(_In_ const Mesh&) = delete;
		/* Move assignment. */
		_Check_return_ Mesh& operator =(_In_ Mesh &&other);

		/* Gets whether the mesh should be indexed rendered. */
		_Check_return_ inline bool IsIndexRendered(void) const
		{
			return idxAcce;
		}

		/* Gets the index accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetIndex(void)
		{
			return GetAccessor(idxAcce, "index");
		}

		/* Gets the position accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetPosition(void)
		{
			return GetAccessor(pos, "position");
		}

		/* Gets the normal accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetNormal(void)
		{
			return GetAccessor(norm, "normal");
		}

		/* Gets the tangent accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetTangent(void)
		{
			return GetAccessor(tang, "tangent");
		}

		/* Gets the first texture coordinate accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetTexcoord(void)
		{
			return GetAccessor(tex1, "texture uv");
		}

		/* Gets the second texture coordinate accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetTexcoord2(void)
		{
			return GetAccessor(tex2, "second texture uv");
		}

		/* Gets the vertex color accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetColor(void)
		{
			return GetAccessor(clr, "vertex color");
		}

		/* Gets the joints accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetJoints(void)
		{
			return GetAccessor(jnts, "joints");
		}

		/* Gets the weights accessor associated with the mesh. */
		_Check_return_ inline BufferAccessor& GetWeights(void)
		{
			return GetAccessor(wghs, "weights");
		}

	private:
		friend void RelocateData(const GLTFFile&, byte**, const vector<std::reference_wrapper<Buffer>>&, vector<Mesh*>&);
		PrimitiveTopology renderMode;

		BufferView *idxView;
		BufferAccessor *idxAcce;
		BufferAccessor *pos;
		BufferAccessor *norm;
		BufferAccessor *tang;
		BufferAccessor *tex1;
		BufferAccessor *tex2;
		BufferAccessor *clr;
		BufferAccessor *jnts;
		BufferAccessor *wghs;

		Mesh(Buffer &buffer, size_t offset, size_t size, size_t stride, PrimitiveTopology topology);

		inline BufferAccessor& GetAccessor(BufferAccessor *accessor, const char *type)
		{
			if (!accessor) Log::Fatal("Mesh doesn't contain a %s buffer!", type);
			return *accessor;
		}

		void Destroy(void);
	};
}