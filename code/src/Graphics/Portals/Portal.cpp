#include "Graphics\Portals\Portal.h"
#include "GameLogic\EuclidRoom.h"

using namespace Plutonium;

Plutonium::Portal::~Portal(void)
{
	delete_s(mesh);
}

Plutonium::Portal::Portal(Mesh * mesh, WindowHandler wnd)
	: WorldObject(), mesh(mesh), center()
{
	/* Calculate portal center. */
	for (size_t i = 0; i < mesh->GetVertexCount(); i++) center += mesh->GetVertexAt(i).Position;
	center /= static_cast<float>(mesh->GetVertexCount());

	mesh->Finalize(wnd);
}

Matrix Plutonium::Portal::GetInverseView(const Matrix & view)
{
	const Matrix &model = GetWorld();
	return view * model * Matrix::CreateRotation(PI, normalize(model.GetUp())) * Destination->GetWorld().GetInverse();
}

/*
Lengyel, Eric. "Oblique View Frustum Depth Projection and Clipping".
Journal of Game Development, Vol. 1, No. 2 (2005)
http://www.terathon.com/code/oblique.html
*/
Matrix Plutonium::Portal::GetClippedProjection(const Matrix & view, const Matrix & proj)
{
	/* Calculate the required clipping plane. */
	float distance = center.Length();
	Vector4 clipPlane = Vector4(normalize(GetWorld().GetForward()), distance);
	clipPlane = view.GetTranspose().GetInverse() * clipPlane;

	/* If the camera is on the positive side of the plane, we don't need to do anything. */
	if (clipPlane.W > 0.0f) return proj;

	/* The the underlying matrix values for fast conversion. */
	const float *matrix = proj.GetComponents();

	/* Calculate the clip-space corner point opposite the clipping plane. */
	Vector4 q(
		(sign(clipPlane.X) + matrix[8]) / matrix[0],
		(sign(clipPlane.Y) + matrix[9]) / matrix[5],
		-1.0f,
		(1.0f + matrix[10]) / matrix[14]);

	/* Calculate the scaled plane vector. */
	Vector4 c = clipPlane * (2.0f / dot(clipPlane, q));
	c.Z += 1.0f;

	/* Replacethe third row of te projection matrix. */
	Matrix result = proj;
	_CrtSetRow<2>(result, c);
	return result;
}