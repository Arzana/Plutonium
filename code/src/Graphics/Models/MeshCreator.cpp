#include "Graphics/Models/MeshCreator.h"
#include "Core/Diagnostics/NotImplementedException.h"

void Pu::MeshCreator::CreateRectangle(BufferAccessor * positions, BufferAccessor * texCoords, const Matrix & transform)
{
	/* Only attempt to set the positions if a valid accessor is specified. */
	if (positions)
	{
		if (positions->GetElementType() == FieldTypes::Vec2)
		{
			/* Convert the positions using the transform and convert back to 2D. */
			static Vector2 data[] =
			{
				(transform * Vector3(-1.0f, -1.0f, 1.0f)).XY,
				(transform * Vector3(-1.0f, 1.0f, 1.0f)).XY,
				(transform * Vector3(1.0f, -1.0f, 1.0f)).XY,
				(transform * Vector3(1.0f, 1.0f, 1.0f)).XY
			};

			positions->SetData(positions, 4);
		}
		else InvalidAccessor(*positions, FieldTypes::Vec2);
	}

	/* Only attempt to set the texture coordinates if a valid accessor is specified. */
	if (texCoords)
	{
		if (texCoords->GetElementType() == FieldTypes::Vec2)
		{
			/* Texture coordinates don't change with transformation so just keep them as raw data. */
			static Vector2 data[] =
			{
				Vector2(0.0f, 0.0f),
				Vector2(0.0f, 1.0f),
				Vector2(1.0f, 0.0f),
				Vector2(1.0f, 1.0f)
			};

			texCoords->SetData(data, 4);
		}
		else InvalidAccessor(*texCoords, FieldTypes::Vec2);
	}
}

void Pu::MeshCreator::CreatePlane(BufferAccessor * /*positions*/, BufferAccessor * /*normals*/, BufferAccessor * /*texCoords*/, BufferAccessor * /*tangents*/, const Matrix & /*transform*/)
{
	throw NotImplementedException(typeid(MeshCreator::CreatePlane));
}

void Pu::MeshCreator::CreateBox(BufferAccessor * positions, BufferAccessor * /*normals*/, BufferAccessor * /*texCoords*/, BufferAccessor * /*tangents*/, const Matrix & /*transform*/)
{
	/* Only attempt to set the positions if a valid accessor is specified. */
	if (positions)
	{
		if (positions->GetElementType() == FieldTypes::Vec3)
		{
			static Vector3 data[] =
			{
				Vector3(-1.0f, 1.0f, 1.0f),
				Vector3(1.0f, -1.0f, -1.0f),
				Vector3(-1.0f, -1.0f, -1.0f),
				Vector3(1.0f, -1.0f, -1.0f),
				Vector3(-1.0f, 1.0f, -1.0f),
				Vector3(1.0f, 1.0f, -1.0f),
				Vector3(-1.0f, -1.0f, 1.0f),
				Vector3(-1.0f, 1.0f, -1.0f),
				Vector3(-1.0f, -1.0f, -1.0f),
				Vector3(-1.0f, 1.0f, -1.0f),
				Vector3(-1.0f, -1.0f, 1.0f),
				Vector3(-1.0f, 1.0f, 1.0f),
				Vector3(1.0f, -1.0f, -1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(1.0f, -1.0f, 1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(1.0f, -1.0f, -1.0f),
				Vector3(1.0f, 1.0f, -1.0f),
				Vector3(-1.0f, -1.0f, 1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(-1.0f, 1.0f, 1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(-1.0f, -1.0f, 1.0f),
				Vector3(1.0f, -1.0f, 1.0f),
				Vector3(-1.0f, 1.0f, -1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(1.0f, 1.0f, -1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(-1.0f, 1.0f, -1.0f),
				Vector3(-1.0f, 1.0f, 1.0f),
				Vector3(-1.0f, -1.0f, -1.0f),
				Vector3(1.0f, -1.0f, -1.0f),
				Vector3(-1.0f, -1.0f, 1.0f),
				Vector3(1.0f, -1.0f, -1.0f),
				Vector3(1.0f, -1.0f, 1.0f),
				Vector3(-1.0f, -1.0f, 1.0f)
			};

			positions->SetData(data, 36);
		}
	}
}

void Pu::MeshCreator::CreateSphere(BufferAccessor * /*positions*/, BufferAccessor * /*normals*/, BufferAccessor * /*texCoords*/, BufferAccessor * /*tangents*/, size_t /*meridians*/, size_t /*parallels*/, const Matrix & /*transform*/)
{
	throw NotImplementedException(typeid(MeshCreator::CreateSphere));
}

void Pu::MeshCreator::CreatePyramid(BufferAccessor * /*positions*/, BufferAccessor * /*normals*/, BufferAccessor * /*texCoords*/, BufferAccessor * /*tangents*/, const Matrix & /*transform*/)
{
	throw NotImplementedException(typeid(MeshCreator::CreatePyramid));
}

void Pu::MeshCreator::InvalidAccessor(const BufferAccessor & accessor, FieldTypes requiredType)
{
	Log::Fatal("Attempting to pass '%s' accessor as '%s' accessor!", to_string(accessor.GetElementType()), to_string(requiredType));
}