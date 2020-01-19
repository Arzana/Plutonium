#include "Graphics/Lighting/CameraUniformBlock.h"

Pu::CameraUniformBlock::CameraUniformBlock(DescriptorPool & pool)
	: UniformBlock(pool, false), exposure(1.0f), brightness(0.0f), contrast(1.0f)
{
	binding1 = pool.GetSubpass(1).GetDescriptor("IProjection").GetAllignedOffset(sizeof(Matrix) << 1);
	binding2 = pool.GetSubpass(2).GetDescriptor("Exposure").GetAllignedOffset((sizeof(Matrix) << 2) + sizeof(Vector3));
}

void Pu::CameraUniformBlock::SetProjection(const Matrix & value)
{
	proj = value;
	iproj = value.GetInverse();
	IsDirty = true;
}

void Pu::CameraUniformBlock::SetView(const Matrix & value)
{
	view = value;
	iview = value.GetInverse();
	IsDirty = true;
}

void Pu::CameraUniformBlock::Stage(byte * dest)
{
	/* Binding 0 (G-Pass). */
	Copy(dest, &proj);
	Copy(dest + sizeof(Matrix), &view);

	/* Binding 1 (Light-Pass). */
	Copy(dest + binding1, &iproj);
	Copy(dest + binding1 + sizeof(Matrix), &iview);
	Copy(dest + binding1 + (sizeof(Matrix) << 1), &pos);

	/* Binding 2 (Tonemap-Pass). */
	Copy(dest + binding2, &exposure);
	Copy(dest + binding2 + sizeof(float), &brightness);
	Copy(dest + binding2 + (sizeof(float) << 1), &contrast);
}