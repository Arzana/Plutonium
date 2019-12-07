#include "CompileToPum.h"
#include "CompileFromMd2.h"
#include "CompileFromGltf.h"
#include "CompileFromObj.h"
#include <Content/AssetSaver.h>
#include <Streams/FileWriter.h>
#include <Streams/BinaryWriter.h>
#include <Graphics/Resources/ImageHandler.h>

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

	/* The base offset depends on which offsets are even writen. */
	size_t baseOffset = writer.GetSize() + sizeof(size_t) * 2;
	if (data.Nodes.size()) baseOffset += sizeof(size_t);
	if (data.Geometry.size()) baseOffset += sizeof(size_t);
	if (data.Animations.size()) baseOffset += sizeof(size_t);
	if (data.Skeletons.size()) baseOffset += sizeof(size_t);
	if (data.Materials.size()) baseOffset += sizeof(size_t);
	if (data.Textures.size()) baseOffset += sizeof(size_t);

	/* Start by writing part of the header the offsets will be added once we go over the items themselves. */
	file.Write(writer.GetData(), 0, writer.GetSize());
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
			if (mesh.IndexMode != 2)
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
			writer.Write(material.SpecularPower);
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

void ConvertMaterials(PumIntermediate &data, const CLArgs &args)
{
	const wstring dir = args.Output.fileDirectory().toWide() + args.DisplayName.toWide() + L"\\";
	const ustring longDir = dir.toUTF32();

	/* Log the operation we're about to do to inform the user, also just copy over the other textures. */
	size_t cnt = 0;
	for (pum_texture &texture : data.Textures)
	{
		cnt += !texture.ConversionCount;
		if (texture.ConversionCount)
		{
			const wstring path = texture.Identifier.toWide();
			FileWriter::CopyFile(path, dir + path.fileName());
			texture.Identifier = longDir + texture.Identifier.fileName();
		}
	}

	/* Early out. */
	if (cnt == 0) return;
	Log::Message("Converting %zu textures from MetalRoughness to SpecularGlossiness!", cnt);

	/*
	We must convert the textures with their materials taken into account.
	This is because the conversion method used the metalness of the material as a parameter.
	*/
	for (pum_material &material : data.Materials)
	{
		/* Skip materials that don't need convertion. */
		if (material.IsFinalized) continue;

		vector<byte> albedoData, metalData;
		ImageInformation albedoInfo, metalInfo;

		/* Load the data for the albedo texture if needed. */
		const bool useAlbedoUV = material.HasDiffuseTexture;
		bool useAlpha = false, setDiffuse = false;
		if (useAlbedoUV)
		{
			const wstring path = data.Textures[material.DiffuseTexture].Identifier.toWide();
			albedoData = std::move(_CrtLoadImageLDR(path, albedoInfo));
			useAlpha = albedoInfo.Components > 3;
			setDiffuse = !data.Textures[material.DiffuseTexture].ConversionCount;
		}

		/* Load the data for the metal texture if needed. */
		const bool useMetalUV = material.HasSpecularGlossTexture;
		bool setSpecGloss = false;
		if (useMetalUV)
		{
			const wstring path = data.Textures[material.SpecGlossTexture].Identifier.toWide();
			metalData = std::move(_CrtLoadImageLDR(path, metalInfo));
			setSpecGloss = !data.Textures[material.SpecGlossTexture].ConversionCount;
		}

		/* Make sure that the textures are the same size, might implement logic to deal with this later. */
		if (useAlbedoUV && useMetalUV)
		{
			if (albedoInfo.Width != metalInfo.Width || albedoInfo.Height != metalInfo.Height)
			{
				Log::Error("Cannnot currently handle metal/roughness and albedo textures of different sizes!");
				return;
			}
		}

		const size_t pixelCnt = albedoInfo.Width * albedoInfo.Height;
		const size_t bufferSize = pixelCnt * sizeof(Color);
		const Color black = Color::Black();
		const Color white = Color::White();
		const Color dieletric{ 0.04f, 0.04f, 0.04f };

		/* We always need to create a specular texture, but the diffuse texture only has to be created if a albedo texture is present. */
		Color *diffuseData = setDiffuse ? reinterpret_cast<Color*>(malloc(bufferSize)) : nullptr;
		Color *specData = setSpecGloss ? reinterpret_cast<Color*>(malloc(bufferSize)) : nullptr;

		/* Metalness is stored in the G channel and roughness in B so start at 1. */
		for (size_t i = 0, j = 0, k = 0; i < pixelCnt; i++, j += sizeof(Color), k += sizeof(Color))
		{
			/* Get the values for the calculation. */
			const Color albedo = useAlbedoUV ? Color(albedoData[j], albedoData[j + 1], albedoData[j + 2]) : white;
			const float metal = useMetalUV ? static_cast<float>(metalData[k + 2]) / 255.0f : 0.0f;
			const float roughness = useMetalUV ? static_cast<float>(metalData[k + 1]) / 225.0f : 0.0f;

			/* Get the diffuse and specular color, copy over the alpha. */
			const Color diffuse = setDiffuse ? Color::Lerp(albedo, black, metal) : albedo;
			const Color specular = setSpecGloss ? Color::Lerp(dieletric, albedo, metal) : (useMetalUV ? Color(metalData[k], metalData[k + 1], metalData[k + 2]) : dieletric);
			const byte alpha = useAlbedoUV && useAlpha ? albedoData[j + 3] : 255;
			const float glossiness = setSpecGloss ? 1.0f - roughness : roughness;

			if (setDiffuse) diffuseData[i] = Color(diffuse.R, diffuse.G, diffuse.B, alpha);
			if (setSpecGloss) specData[i] = Color(specular.R, specular.G, specular.B, static_cast<byte>(glossiness * 255.0f));
		}

		/* Save the new diffuse texture if needed. */
		if (setDiffuse)
		{
			pum_texture &diffuse = data.Textures[material.DiffuseTexture];

			/* The new identifier for the texture is its origional name but in the output directory and as a png. */
			diffuse.ConversionCount++;
			const wstring path = dir + diffuse.Identifier.toWide().fileNameWithoutExtension();
			AssetSaver::SaveImage(diffuseData, albedoInfo.Width, albedoInfo.Height, Format::R8G8B8A8_UNORM, path, ImageSaveFormats::Png);
			diffuse.Identifier = longDir + diffuse.Identifier.fileNameWithoutExtension() + U".png";

			free(diffuseData);
		}

		/* Save the specular glossiness texture. */
		if (setSpecGloss)
		{
			pum_texture &specGloss = data.Textures[material.SpecGlossTexture];

			/* The new identifier for the texture is its origional name but in the output directory and as a png. */
			specGloss.ConversionCount++;
			const wstring path = dir + specGloss.Identifier.toWide().fileNameWithoutExtension();
			AssetSaver::SaveImage(specData, metalInfo.Width, metalInfo.Height, Format::R8G8B8A8_UNORM, path, ImageSaveFormats::Png);
			specGloss.Identifier = longDir + specGloss.Identifier.fileNameWithoutExtension() + U".png";

			free(specData);
		}

		material.IsFinalized = true;
	}
}

int CompileToPum(const CLArgs & args)
{
	const string ext = args.Input.fileExtension().toUpper();
	PumIntermediate data;

	if (ext == "MD2")
	{
		Md2LoaderResult raw;

		LoadMd2(args, raw);
		Md2ToPum(args, raw, data);
	}
	else if (ext == "GLTF" || ext == "GLB")
	{
		GLTFLoaderResult raw;

		LoadGLTF(args, raw);
		GltfToPum(args, raw, data);
	}
	else if (ext == "OBJ")
	{
		ObjLoaderResult raw;

		LoadObjMtl(args.Input, raw);
		ObjToPum(raw, data);
	}
	else
	{
		Log::Error("Invalid file extension passed to compile to .pum!");
		return EXIT_FAILURE;
	}

	ConvertMaterials(data, args);
	return SavePumToFile(args, data);
}