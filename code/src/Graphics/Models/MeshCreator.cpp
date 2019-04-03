#include "Graphics/Models/MeshCreator.h"
#include "Core/Diagnostics/NotImplementedException.h"

Pu::FieldType Pu::MeshCreator::texCoordType = Pu::FieldType(Pu::ComponentType::Float, Pu::SizeType::Vector2);
Pu::FieldType Pu::MeshCreator::pos3DType = Pu::FieldType(Pu::ComponentType::Float, Pu::SizeType::Vector3);

void Pu::MeshCreator::CreateRectangle(BufferAccessor * positions, BufferAccessor * texCoords, const Matrix & transform)
{
	/* Only attempt to set the positions if a valid accessor is specified. */
	if (positions)
	{
		if (positions->GetElementType() == pos3DType)
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
		else InvalidAccessor(*positions, pos3DType);
	}

	/* Only attempt to set the texture coordinates if a valid accessor is specified. */
	if (texCoords)
	{
		if (texCoords->GetElementType() == texCoordType)
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
		else InvalidAccessor(*texCoords, texCoordType);
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
		if (positions->GetElementType() == pos3DType)
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
		else InvalidAccessor(*positions, pos3DType);
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

void Pu::MeshCreator::InvalidAccessor(const BufferAccessor & accessor, FieldType requiredType)
{
	Log::Fatal("Attempting to pass '%s' accessor as '%s' accessor!", accessor.GetElementType().GetName().c_str(), requiredType.GetName().c_str());
}