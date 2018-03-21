#include "Graphics\Portals\Portal.h"

using namespace Plutonium;

Plutonium::Portal::~Portal(void)
{
	delete_s(mesh);
}

Plutonium::Portal::Portal(Vector3 position)
	: WorldObject()
{
	WorldObject::Teleport(position);

	/* Create temporary frame. */
	mesh = new Mesh("PortalFrame");
	mesh->SetBufferSize(6);
	mesh->GetVertexAt(0).Position = Vector3(-0.5f, -0.5f, 0.0f);
	mesh->GetVertexAt(1).Position = Vector3(0.5f, -0.5f, 0.0f);
	mesh->GetVertexAt(2).Position = Vector3(0.5f, 0.5f, 0.0f);
	mesh->GetVertexAt(3).Position = Vector3(0.5f, 0.5f, 0.0f);
	mesh->GetVertexAt(4).Position = Vector3(-0.5f, 0.5f, 0.0f);
	mesh->GetVertexAt(5).Position = Vector3(-0.5f, -0.5f, 0.0f);
	mesh->Finalize();
}

Matrix Plutonium::Portal::GetInverseView(const Matrix & view)
{
	const Matrix &model = GetWorld();
	return view * model * Matrix::CreateRotationY(PI) * model.GetOrientation() * Destination->GetWorld().GetInverse();
}

/*
Lengyel, Eric. "Oblique View Frustum Depth Projection and Clipping".
Journal of Game Development, Vol. 1, No. 2 (2005)
http://www.terathon.com/code/oblique.html
*/
Matrix Plutonium::Portal::GetClippedProjection(const Matrix & view, const Matrix & proj)
{
	float distance = GetPosition().Length();
	Vector4 clipPlane = Vector4(GetWorld().GetOrientation() * Vector3::Forward, distance);
	clipPlane = view.GetTranspose().GetInverse() * clipPlane;

	if (clipPlane.W > 0.0f) return proj;

	Vector4 q = proj.GetInverse() * Vector4(sign(clipPlane.X), sign(clipPlane.Y), 1.0f, 1.0f);
	Vector4 c = clipPlane * (2.0f / dot(clipPlane, q));
	Matrix result = proj;

	_CrtSetRow<2>(result, c - _CrtGetRow<3>(result));
	return result;
}