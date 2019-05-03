#include "CompileToPum.h"
#include "CompileFromMd2.h"
#include <Streams/FileWriter.h>
#include <Streams/BinaryWriter.h>

using namespace Pu;

int SavePumToFile(const CLArgs &args, const PumIntermediate &data)
{
	FileWriter file(args.Output.toWide());
	const size_t dataSize = data.Data.GetSize();

	/* .pum is always saved using little endian. */
	BinaryWriter writer(dataSize, Endian::Little);
	writer.Write(Stream::GetMagicNum("PUM0"));
	writer.Write(makeVersion(1, 0, 0));
	writer.Write(args.DisplayName.toUTF32());

	writer.Write(static_cast<uint32>(data.Nodes.size()));
	writer.Write(static_cast<uint32>(data.Geometry.size()));
	writer.Write(static_cast<uint32>(data.Animations.size()));
	writer.Write(static_cast<uint32>(data.Skeletons.size()));
	writer.Write(static_cast<uint32>(data.Materials.size()));
	writer.Write(static_cast<uint32>(data.Textures.size()));

	/* Start by writing part of the header the offsets will be added once we go over the items themselves. */
	file.Write(writer.GetData(), 0, writer.GetSize());
	const size_t baseOffset = writer.GetSize() + sizeof(size_t) * 8;
	writer.Reset();

	size_t offset = baseOffset;
	if (data.Nodes.size())
	{
		file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));

		for (const pum_node &node : data.Nodes)
		{
			writer.Write(static_cast<uint32>(node.Children.size()));
			for (uint32 i : node.Children) writer.Write(i);
			writer.Write(node.GetFlags());

			if (node.WriteMeshIndex) writer.Write(node.Mesh);
			if (node.WriteSkinIndex) writer.Write(node.Skin);

			if (node.WriteTranslation) writer.Write(node.Translation);
			if (node.WriteRotation) writer.Write(node.Rotation);
			if (node.WriteScale) writer.Write(node.Scale);
		}

		offset = baseOffset + writer.GetSize();
	}

	if (data.Geometry.size())
	{
		file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));

		for (const pum_mesh &mesh : data.Geometry)
		{
			writer.Write(mesh.Identifier);
			writer.Write(mesh.GetFlags());
			writer.Write(mesh.Bounds.LowerBound);
			writer.Write(mesh.Bounds.UpperBound);
			writer.Write(mesh.VertexViewStart);
			writer.Write(mesh.VertexViewSize);

			if (mesh.WriteMaterialIndex) writer.Write(mesh.Material);
			if (mesh.IndexMode)
			{
				writer.Write(mesh.IndexViewStart);
				writer.Write(mesh.IndexViewSize);
			}
		}

		offset = baseOffset + writer.GetSize();
	}

	if (data.Animations.size())
	{
		file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));

		for (const pum_animation &anim : data.Animations)
		{
			writer.Write(anim.Identifier);
			writer.Write(anim.GetFlags());

			if (anim.InterpolationMode > 1)
			{
				writer.Write(anim.Arg1);
				writer.Write(anim.Arg2);
			}

			if (anim.Type)
			{
				writer.Write(static_cast<uint32>(anim.Frames.size()));
				for (uint32 i : anim.Frames) writer.Write(i);
				writer.Write(anim.Duration);
			}
			else
			{
				writer.Write(static_cast<uint32>(anim.Sequences.size()));
				for (const pum_sequency &seq : anim.Sequences)
				{
					writer.Write(seq.Node);
					writer.Write(static_cast<uint32>(seq.Frames.size()));
					for (const pum_frame &frame : seq.Frames)
					{
						writer.Write(frame.Time);
						writer.Write(frame.Translation);
						writer.Write(frame.Rotation);
						writer.Write(frame.Scale);
						writer.Write(frame.Bounds.LowerBound);
						writer.Write(frame.Bounds.UpperBound);
					}
				}
			}
		}

		offset = baseOffset + writer.GetSize();
	}

	if (data.Skeletons.size())
	{
		file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));

		for (const pum_skeleton &skeleton : data.Skeletons)
		{
			writer.Write(skeleton.Identifier);
			writer.Write(skeleton.Root);
			writer.Write(static_cast<uint32>(skeleton.Joints.size()));

			for (const pum_joint &joint : skeleton.Joints)
			{
				writer.Write(joint.Node);
				writer.Write(joint.IBind);
			}
		}

		offset = baseOffset + writer.GetSize();
	}

	if (data.Materials.size())
	{
		file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));

		for (const pum_material &material : data.Materials)
		{
			writer.Write(material.Identifier);
			writer.Write(material.GetFlags());
			writer.Write(material.DiffuseFactor.Packed);
			writer.Write(material.SpecularFactor.Packed);
			writer.Write(material.EmissiveFactor.Packed);
			writer.Write(material.Glossiness);
			writer.Write(material.EmissiveInternsity);

			if (material.AlphaMode == 1) writer.Write(material.AlphaThreshold);
			if (material.HasDiffuseTexture) writer.Write(material.DiffuseTexture);
			if (material.HasSpecularGlossTexture) writer.Write(material.SpecGlossTexture);
			if (material.HasNormalTexture) writer.Write(material.NormalTexture);
			if (material.HasOcclusionTexture) writer.Write(material.OcclusionTexture);
			if (material.HasEmissiveTexture) writer.Write(material.EmissiveTexture);
		}

		offset = baseOffset + writer.GetSize();
	}

	if (data.Textures.size())
	{
		file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));

		for (const pum_texture &texture : data.Textures)
		{
			writer.Write(texture.Identifier);
			writer.Write(texture.GetFlags());
		}

		offset = baseOffset + writer.GetSize();
	}

	/* Write the last pieces of the header to the file. */
	file.Write(reinterpret_cast<byte*>(&offset), 0, sizeof(size_t));
	file.Write(reinterpret_cast<const byte*>(&dataSize), 0, sizeof(size_t));
	file.Write(writer.GetData(), 0, writer.GetSize());
	file.Write(data.Data.GetData(), 0, data.Data.GetSize());

	return EXIT_SUCCESS;
}

int CompileToPum(const CLArgs & args)
{
	const string ext = args.Input.fileExtension().toUpper();
	PumIntermediate data;

	if (ext == "MD2")
	{
		Md2LoaderResult raw;

		LoadMd2(args.Input, raw);
		Md2ToPum(raw, data);
	}
	else
	{
		Log::Error("Invalid file extension passed to compile to .pum!");
		return EXIT_FAILURE;
	}

	return SavePumToFile(args, data);
}