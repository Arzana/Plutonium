#include "Graphics\Models\Shapes.h"

void Plutonium::ShapeCreator::MakePlane(Mesh * mesh, Vector2 scale)
{
	mesh->SetBufferSize(6);

	VertexFormat *vrtx = mesh->GetVertexAt(0);
	vrtx->Position = Vector3(-scale.X, scale.Y, 0.0f);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(1);
	vrtx->Position = Vector3(-scale.X, -scale.Y, 0.0f);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(2);
	vrtx->Position = Vector3(scale.X, -scale.Y, 0.0f);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(3);
	vrtx->Position = Vector3(scale.X, -scale.Y, 0.0f);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(4);
	vrtx->Position = Vector3(scale.X, scale.Y, 0.0f);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(5);
	vrtx->Position = Vector3(-scale.X, scale.Y, 0.0f);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::One();

	SetTangents(mesh);
}

void Plutonium::ShapeCreator::MakeBox(Mesh * mesh, Vector3 scale)
{
	mesh->SetBufferSize(36);

	VertexFormat *vrtx = mesh->GetVertexAt(0);
	vrtx->Position = Vector3(-scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(1);
	vrtx->Position = Vector3(-scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(2);
	vrtx->Position = Vector3(scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(3);
	vrtx->Position = Vector3(scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(4);
	vrtx->Position = Vector3(scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(5);
	vrtx->Position = Vector3(-scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Forward();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(6);
	vrtx->Position = Vector3(-scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Left();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(7);
	vrtx->Position = Vector3(-scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Left();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(8);
	vrtx->Position = Vector3(-scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Left();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(9);
	vrtx->Position = Vector3(-scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Left();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(10);
	vrtx->Position = Vector3(-scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Left();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(11);
	vrtx->Position = Vector3(-scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Left();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(12);
	vrtx->Position = Vector3(scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Right();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(13);
	vrtx->Position = Vector3(scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Right();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(14);
	vrtx->Position = Vector3(scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Right();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(15);
	vrtx->Position = Vector3(scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Right();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(16);
	vrtx->Position = Vector3(scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Right();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(17);
	vrtx->Position = Vector3(scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Right();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(18);
	vrtx->Position = Vector3(-scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Backward();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(19);
	vrtx->Position = Vector3(-scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Backward();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(20);
	vrtx->Position = Vector3(scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Backward();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(21);
	vrtx->Position = Vector3(scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Backward();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(22);
	vrtx->Position = Vector3(scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Backward();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(23);
	vrtx->Position = Vector3(-scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Backward();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(24);
	vrtx->Position = Vector3(-scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Up();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(25);
	vrtx->Position = Vector3(scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Up();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(26);
	vrtx->Position = Vector3(scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Up();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(27);
	vrtx->Position = Vector3(scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Up();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(28);
	vrtx->Position = Vector3(-scale.X, scale.Y, scale.Z);
	vrtx->Normal = Vector3::Up();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(29);
	vrtx->Position = Vector3(-scale.X, scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Up();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(30);
	vrtx->Position = Vector3(-scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(31);
	vrtx->Position = Vector3(-scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(32);
	vrtx->Position = Vector3(scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(33);
	vrtx->Position = Vector3(scale.X, -scale.Y, -scale.Z);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(34);
	vrtx->Position = Vector3(-scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(35);
	vrtx->Position = Vector3(scale.X, -scale.Y, scale.Z);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::UnitX();

	SetTangents(mesh);
}

void Plutonium::ShapeCreator::MakeSphere(Mesh * mesh, size_t meridians, size_t parallels, float radius)
{
	/* Set mesh buffer to the required size. */
	mesh->SetBufferSize(meridians * 6 + (parallels - 2) * meridians * 6);
	std::vector<VertexFormat> vertices;

	/* Pre-calculate spacing for parallels and meridians. */
	float parallelSpacing = recip(static_cast<float>(parallels + 1));
	float meridianSpacing = recip(static_cast<float>(meridians - 1));

	/* Create north pole (set tangent to a default value). */
	vertices.push_back({ Vector3(0.0f, 1.0f, 0.0f), Vector3::Up(), Vector3::Zero(), Vector2::UnitY() });

	/* Create all vertices. */
	for (size_t i = 0; i < parallels; i++)
	{
		for (size_t j = 0; j < meridians; j++)
		{
			/* Get the texture uv for this point. */
			Vector2 uv(static_cast<float>(j) * meridianSpacing, 1.0f - (i + 1) * parallelSpacing);

			/* Get the spherical coordinates, theta (meridian angle) and phi (parallel angle). */
			float theta = uv.X * TAU;
			float phi = (uv.Y - 0.5f) * PI;

			/* The scalar for the line at the equator. */
			float c = cosf(phi);

			/* Get the position and normalize it to get the normal at that point. */
			Vector3 pos = Vector3(c * cosf(theta), sinf(phi), c * sinf(theta)) * radius;
			Vector3 norm = normalize(pos);

			/* Push vertex (set tangent to a default value). */
			vertices.push_back({ pos, norm, Vector3::Zero(), uv });
		}
	}

	/* Create south pole (set tangent to a default value). */
	vertices.push_back({ Vector3(0.0f, -1.0f, 0.0f), Vector3::Down(), Vector3::Zero(), Vector2::Zero() });

	/* Convert the vertices of the north pole to triangles and push them to the mesh. */
	size_t k = 0;
	for (size_t i = 0; i < meridians; i++)
	{
		size_t b = i + 1;
		size_t c = (i + 1) % meridians + 1;

		*mesh->GetVertexAt(k++) = vertices.at(0);
		*mesh->GetVertexAt(k++) = vertices.at(b);
		*mesh->GetVertexAt(k++) = vertices.at(c);
	}

	/* Convert the vertices of the normal positions to quads and push them to the mesh. */
	for (size_t i = 0; i < parallels - 2; i++)
	{
		size_t aStart = i * meridians + 1;
		size_t bStart = (i + 1) * meridians + 1;

		for (size_t j = 0; j < meridians; j++)
		{
			size_t a = aStart + j;
			size_t b = aStart + (j + 1) % meridians;
			size_t c = bStart + j;
			size_t d = bStart + (j + 1) % meridians;

			*mesh->GetVertexAt(k++) = vertices.at(a);
			*mesh->GetVertexAt(k++) = vertices.at(b);
			*mesh->GetVertexAt(k++) = vertices.at(d);
			*mesh->GetVertexAt(k++) = vertices.at(a);
			*mesh->GetVertexAt(k++) = vertices.at(c);
			*mesh->GetVertexAt(k++) = vertices.at(d);
		}
	}

	/* Convert the vertices of the south pole to triangles and push them to the mesh. */
	for (size_t i = 0; i < meridians; i++)
	{
		size_t a = vertices.size() - 1;
		size_t b = i + meridians * (parallels - 2) + 1;
		size_t c = (i + 1) % meridians + meridians * (parallels - 2) + 1;
		
		*mesh->GetVertexAt(k++) = vertices.at(a);
		*mesh->GetVertexAt(k++) = vertices.at(b);
		*mesh->GetVertexAt(k++) = vertices.at(c);
	}

	/* Set the tangents for all vertexes in the mesh. */
	SetTangents(mesh);
}

void Plutonium::ShapeCreator::MakePyramid(Mesh * mesh, Vector2 base, float height)
{
	mesh->SetBufferSize(18);

	/* Base plate. */
	VertexFormat *vrtx = mesh->GetVertexAt(0);
	vrtx->Position = Vector3::Zero();
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::Zero();

	vrtx = mesh->GetVertexAt(1);
	vrtx->Position = Vector3::UnitX() * base.X;
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(2);
	vrtx->Position = Vector3::UnitZ() * base.Y;
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::UnitY();

	vrtx = mesh->GetVertexAt(3);
	vrtx->Position = Vector3::UnitX() * base.X;
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::UnitX();

	vrtx = mesh->GetVertexAt(4);
	vrtx->Position = Vector3(base.X, 0.0f, base.Y);
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::One();

	vrtx = mesh->GetVertexAt(5);
	vrtx->Position = Vector3::UnitZ() * base.Y;
	vrtx->Normal = Vector3::Down();
	vrtx->Texture = Vector2::UnitY();

	/* Side. */
	vrtx = mesh->GetVertexAt(6);
	vrtx->Position = Vector3::Zero();
	vrtx->Texture = Vector2::Zero();

	VertexFormat *vrtx1 = mesh->GetVertexAt(7);
	vrtx1->Position = Vector3(base.X * 0.5f, height, base.Y * 0.5f);
	vrtx1->Texture = Vector2(0.5f, 1.0f);

	VertexFormat *vrtx2 = mesh->GetVertexAt(8);
	vrtx2->Position = Vector3(base.X, 0.0f, 0.0f);
	vrtx2->Texture = Vector2::UnitX();

	Mesh::SetNormal(*vrtx, *vrtx1, *vrtx2);

	/* Side. */
	vrtx = mesh->GetVertexAt(9);
	vrtx->Position = Vector3::Zero();
	vrtx->Texture = Vector2::Zero();

	vrtx1 = mesh->GetVertexAt(10);
	vrtx1->Position = Vector3(base.X * 0.5f, height, base.Y * 0.5f);
	vrtx1->Texture = Vector2(0.5f, 1.0f);

	vrtx2 = mesh->GetVertexAt(11);
	vrtx2->Position = Vector3::UnitZ() * base.Y;
	vrtx2->Texture = Vector2::UnitX();

	Mesh::SetNormal(*vrtx1, *vrtx, *vrtx2);

	/* Side. */
	vrtx = mesh->GetVertexAt(12);
	vrtx->Position = Vector3::UnitZ() * base.Y;
	vrtx->Texture = Vector2::Zero();

	vrtx1 = mesh->GetVertexAt(13);
	vrtx1->Position = Vector3(base.X * 0.5f, height, base.Y * 0.5f);
	vrtx1->Texture = Vector2(0.5f, 1.0f);

	vrtx2 = mesh->GetVertexAt(14);
	vrtx2->Position = Vector3(base.X, 0.0f, base.Y);
	vrtx2->Texture = Vector2::UnitX();

	Mesh::SetNormal(*vrtx1, *vrtx, *vrtx2);

	/* Side. */
	vrtx = mesh->GetVertexAt(15);
	vrtx->Position = Vector3(base.X, 0.0f, base.Y);
	vrtx->Texture = Vector2::Zero();

	vrtx1 = mesh->GetVertexAt(16);
	vrtx1->Position = Vector3(base.X * 0.5f, height, base.Y * 0.5f);
	vrtx1->Texture = Vector2(0.5f, 1.0f);

	vrtx2 = mesh->GetVertexAt(17);
	vrtx2->Position = Vector3::UnitX() * base.X;
	vrtx2->Texture = Vector2::UnitX();

	Mesh::SetNormal(*vrtx1, *vrtx, *vrtx2);
	SetTangents(mesh);
}

void Plutonium::ShapeCreator::SetTangents(Mesh * mesh)
{
	for (size_t i = 0; i < mesh->GetVertexCount(); i += 3)
	{
		Mesh::SetTangent(*mesh->GetVertexAt(i), *mesh->GetVertexAt(i + 1), *mesh->GetVertexAt(i + 2));
	}
}