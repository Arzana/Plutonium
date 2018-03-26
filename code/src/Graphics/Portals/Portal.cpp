#include "Graphics\Portals\Portal.h"
#include "GameLogic\EuclidRoom.h"

using namespace Plutonium;

Plutonium::Portal::~Portal(void)
{
	delete_s(mesh);
}

Plutonium::Portal::Portal(Mesh * mesh)
	: WorldObject(), mesh(mesh)
{
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