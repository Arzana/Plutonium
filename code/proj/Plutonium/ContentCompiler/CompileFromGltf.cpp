#include "CompileFromGltf.h"
#include <Streams/FileReader.h>
#include <Streams/BinaryReader.h>
#include <nlohmann/fifo_map.hpp>
#include <nlohmann/json.hpp>
#include "Config.h"

using namespace Pu;

/* GLTF needs the order for it's indeces so make sure the Json parser maintains the order. */
template <typename key_t, typename value_t, typename dummy_compare_t, typename allocator_t>
using ordered_fifo_map = nlohmann::fifo_map<key_t, value_t, nlohmann::fifo_map_compare<key_t>, allocator_t>;
using json = nlohmann::basic_json<ordered_fifo_map, std::vector, std::string, bool, int64_t, size_t, float>;

/* Quick macro to check if a global Json value is an array. */
#define CHECK_IF_ARRAY(value)		if (!value.is_array())											\
									{																\
										LogCorruptJsonHeader(#value " aren't defined in an array");	\
										return;														\
									}

	/* Logs a corrupt GLTF Json header message. */
inline void LogCorruptJsonHeader(const char *reason)
{
	Log::Error("Corrupt GLTF Json header (%s)!", reason);
}

/* Returns true if the asset isn't GLTF version 2.0. */
bool HandleJsonAsset(const json &asset)
{
	/* Object element is always an object. */
	if (!asset.is_object())
	{
		LogCorruptJsonHeader("asset element isn't an object");
		return true;
	}

	bool minimumValueReached = false;
	float version = 0.0f;

	/*
	Asset can contains multiple values:
	version:		The version of this GLTF file.
	minVersion:		The minimum version required to parse this GLTF file.
	generator:		The generator used to create this GLTF file.				(not handled)
	copyright:		Optional copyright.											(not handled)
	*/
	for (const auto &[key, val] : asset.items())
	{
		const string upperKey = string(key).toUpper();
		if (upperKey == "MINVERSION")
		{
			if (!val.is_string())
			{
				LogCorruptJsonHeader("asset minVersion isn't a valid version");
				return true;
			}

			/* Convert the string to the version. */
			const float minVersion = strtof(static_cast<std::string>(val).c_str(), nullptr);

			/* Make sure we can load the file. */
			if (minVersion > MinimumVersionGLTF)
			{
				Log::Error("Unable to load GLTF (minimum required parser version %f is greater than supported parser version %f)!", static_cast<float>(val), MinimumVersionGLTF);
				return true;
			}
			else minimumValueReached = true;
		}
		else if (upperKey == "VERSION")
		{
			if (!val.is_string())
			{
				LogCorruptJsonHeader("asset version isn't a valid version");
				return true;
			}

			/* Save the version for checking later (in case a minimum value is present). */
			version = strtof(static_cast<std::string>(val).c_str(), nullptr);
		}
	}

	/* Check for the direct version if no minimum version is specified. */
	if (!minimumValueReached && version > MinimumVersionGLTF)
	{
		Log::Error("Unable to load GLTF (version %f is greater than parser version %f)!", version, MinimumVersionGLTF);
		return true;
	}

	return false;
}

/* Sets the default scene. */
void HandleJsonDefaultScene(const json &scene, GLTFLoaderResult &file)
{
	/* Scene is a single index value to a scene so simply parse that. */
	if (scene.is_number_unsigned()) file.DefaultScene = scene;
	else LogCorruptJsonHeader("default scene cannot be interpreted as an index");
}

/* Parses all scenes. */
void HandleJsonScenes(const json &scenes, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(scenes);

	/* Loops over the scenes in the array */
	file.Scenes.reserve(scenes.size());
	for (const json &cur : scenes)
	{
		/* Scenes must always be objects. */
		GLTFScene scene;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-scene object found in scenes");
			continue;
		}

		/*
		A scene can contain the following values:
		name:		optional name of the scene.
		nodes:		the root nodes of the scene.
		*/
		for (const auto &[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "NAME")
			{
				/* Name is optional but must be a string. */
				if (val.is_string()) scene.Name = val;
				else LogCorruptJsonHeader("scene name isn't a string");
			}
			else if (upperKey == "NODES")
			{
				/* Nodes are always in an array. */
				if (!val.is_array())
				{
					LogCorruptJsonHeader("scene nodes isn't an array");
					continue;
				}

				/* Resever for less allocation and get the indeces. */
				scene.Nodes.reserve(val.size());
				for (const json &index : val)
				{
					if (index.is_number_unsigned()) scene.Nodes.emplace_back(index);
					else LogCorruptJsonHeader("scene node index is not an unsigned int");
				}
			}
		}

		/* Every scene needs to be added to avoid index errors. */
		file.Scenes.emplace_back(scene);
	}
}

/* Parses the json nodes. */
void HandleJsonNodes(const json &nodes, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(nodes);

	/* Loops over the nodes in the array */
	file.Nodes.reserve(nodes.size());
	for (const json &cur : nodes)
	{
		/* Nodes must always be objects. */
		GLTFNode node;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-node object found in nodes");
			continue;
		}

		/*
		A node can contain the following values:
		name:			the optional name of the node.
		children:		the cild node indeces.
		matrix:			the TRS matrix transform.			(decomposed into T, R and S)
		translation:	the translation vector.
		rotation:		the rotation quaternion.
		scale:			the scalar vector.
		camera:			an associated camera index.			(not handled)
		skin:			an associated skin index.
		mesh:			an associated mesh index.
		*/
		for (const auto&[key, val] : cur.items())
		{
			/* Handle optional name. */
			const string upperKey = string(key).toUpper();
			if (upperKey == "NAME")
			{
				if (val.is_string()) node.Name = val;
				else LogCorruptJsonHeader("node name isn't a string");
			}
			else if (upperKey == "CHILDREN")
			{
				/* Children should be unsigned int array. */
				if (!val.is_array())
				{
					LogCorruptJsonHeader("node children aren't in array");
					continue;
				}

				/* Loop over all child node indeces. */
				node.Children.reserve(val.size());
				for (const json &idx : val)
				{
					if (idx.is_number_unsigned()) node.Children.emplace_back(idx);
					else LogCorruptJsonHeader("node child index isn't unsigned int");
				}
			}
			else if (upperKey == "MATRIX")
			{
				/* Matrix should be a float array of 16 elements. */
				if (!val.is_array() || val.size() != 16)
				{
					LogCorruptJsonHeader("node matrix isn't 4x4 float matrix");
					continue;
				}

				/* Loop over each element in the matrix. */
				float elements[16];
				for (size_t i = 0; i < val.size(); i++)
				{
					const json &cell = val[i];
					if (cell.is_number()) elements[i] = cell;
					else LogCorruptJsonHeader("node matrix element isn't a float");
				}

				/* Convert to matrix structure (both are column-major) and decompose into components. */
				const Matrix matrix = b2mtrx(elements);
				node.Translation = matrix.GetTranslation();
				node.Rotation = matrix.GetOrientation();
				node.Scale = matrix.GetScale();
			}
			else if (upperKey == "TRANSLATION")
			{
				/* Translation should be a float array of 3 elements. */
				if (!val.is_array() || val.size() != 3)
				{
					LogCorruptJsonHeader("node translation isn't a 3D vector");
					continue;
				}

				/* Loop over each element in the vector. */
				for (size_t i = 0; i < val.size(); i++)
				{
					const json &cell = val[i];
					if (cell.is_number()) node.Translation.f[i] = cell;
					else LogCorruptJsonHeader("node translation element isn't a float");
				}
			}
			else if (upperKey == "ROTATION")
			{
				/* Rotation should be a float array of 4 elements. */
				if (!val.is_array() || val.size() != 4)
				{
					LogCorruptJsonHeader("node rotation isn't a quaternion");
					continue;
				}

				/* Loop over each element in the quaternion. */
				float elements[4];
				for (size_t i = 0; i < val.size(); i++)
				{
					const json &cell = val[i];
					if (cell.is_number()) elements[i] = cell;
					else LogCorruptJsonHeader("node rotation element isn't a float");
				}

				/* Convert the elements to a quaternion. */
				node.Rotation = b2quat(elements);
			}
			else if (upperKey == "SCALE")
			{
				/* Scale should be a float array of 3 elements. */
				if (!val.is_array() || val.size() != 3)
				{
					LogCorruptJsonHeader("node scale isn't a 3D vector");
					continue;
				}

				/* Loop over each element in the vector. */
				for (size_t i = 0; i < val.size(); i++)
				{
					const json &cell = val[i];
					if (cell.is_number()) node.Scale.f[i] = cell;
					else LogCorruptJsonHeader("node scale element isn't a float");
				}
			}
			else if (upperKey == "SKIN")
			{
				if (val.is_number_unsigned())
				{
					node.HasSkin = true;
					node.Skin = val;
				}
				else LogCorruptJsonHeader("node skin index isn't an unsigned int");
			}
			else if (upperKey == "MESH")
			{
				if (val.is_number_unsigned())
				{
					node.HasMesh = true;
					node.Mesh = val;
				}
				else LogCorruptJsonHeader("node mesh index ins't an unsigned int");
			}
		}

		file.Nodes.emplace_back(node);
	}
}

void HandleJsonMeshPrimitives(const json &primitives, GLTFMesh &mesh)
{
	CHECK_IF_ARRAY(primitives);

	/* Loop over the primitives in the array. */
	mesh.Primitives.reserve(primitives.size());
	for (const json &cur : primitives)
	{
		/* Mesh primitives must always be objects. */
		GLTFPrimitive primitive;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-primitive object found in mesh primitives");
			continue;
		}

		/*
		A mesh primitive can contain the following values:
		attributes:		the fields of the mesh.
		indices:		an optional index to a indices accessor.
		material:		an optional index to a material.
		mode:			an optional primitive mode (triangles by default).
		*/
		for (const auto &[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "ATTRIBUTES")
			{
				if (!val.is_object())
				{
					LogCorruptJsonHeader("mesh primive attributes isn't an object");
					continue;
				}

				/* Loop over the types and their accessor indices. */
				for (const auto &[type, idx] : val.items())
				{
					const string upperType = string(type).toUpper();
					if (upperType == "POSITION")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::Position, idx);
						else LogCorruptJsonHeader("mesh primitive position isn't a valid index");
					}
					else if (upperType == "NORMAL")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::Normal, idx);
						else LogCorruptJsonHeader("mesh primitive normal isn't a valid index");
					}
					else if (upperType == "TANGENT")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::Tangent, idx);
						else LogCorruptJsonHeader("mesh primitive tangent isn't a valid index");
					}
					else if (upperType == "TEXCOORD_0")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::TexCoord1, idx);
						else LogCorruptJsonHeader("mesh primitive first texture coordinate isn't a valid index");
					}
					else if (upperType == "TEXCOORD_1")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::TexCoord2, idx);
						else LogCorruptJsonHeader("mesh primitive second texture coordinate isn't a valid index");
					}
					else if (upperType == "COLOR_0")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::Color, idx);
						else LogCorruptJsonHeader("mesh primitive color isn't a valid index");
					}
					else if (upperType == "JOINTS_0")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::Joints, idx);
						else LogCorruptJsonHeader("mesh primitive joints isn't a valid index");
					}
					else if (upperType == "WEIGHTS_0")
					{
						if (idx.is_number_unsigned()) primitive.Attributes.emplace(GLTFPrimitiveAttribute::Weights, idx);
						else LogCorruptJsonHeader("mesh primitive wiehgts isn't a valid index");
					}
				}
			}
			else if (upperKey == "INDICES")
			{
				if (val.is_number_unsigned())
				{
					primitive.HasIndices = true;
					primitive.Indices = val;
				}
				else LogCorruptJsonHeader("mesh primitive indices isn't a valid index");
			}
			else if (upperKey == "MATERIAL")
			{
				if (val.is_number_unsigned())
				{
					primitive.HasMaterial = true;
					primitive.Material = val;
				}
				else LogCorruptJsonHeader("mesh primitive material isn't a valid index");
			}
			else if (upperKey == "MODE")
			{
				if (val.is_number_unsigned()) primitive.Mode = static_cast<GLTFMode>(val);
				else LogCorruptJsonHeader("mesh primitive mode isn't a valid enum value");
			}
		}

		mesh.Primitives.emplace_back(primitive);
	}
}

void HandleJsonMeshes(const json &meshes, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(meshes);

	/* Loops over the meshes in the array */
	file.Meshes.reserve(meshes.size());
	for (const json &cur : meshes)
	{
		/* Meshes must always be objects. */
		GLTFMesh mesh;;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-mesh object found in meshes");
			continue;
		}

		/*
		A mesh can contain the following values:
		name:			optional name of the mesh.
		primitives:		data values of the mesh.
		weights:		morph target wieghts.		(not handled)
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "NAME")
			{
				if (val.is_string()) mesh.Name = val;
				else LogCorruptJsonHeader("mesh name isn't a string");
			}
			else if (upperKey == "PRIMITIVES") HandleJsonMeshPrimitives(val, mesh);
		}

		file.Meshes.emplace_back(mesh);
	}
}

void HandleJsonAnimationChannels(const json &channels, GLTFAnimation &animation)
{
	CHECK_IF_ARRAY(channels);

	animation.Channels.reserve(channels.size());
	for (const json &cur : channels)
	{
		GLTFChannel channel;
		bool nodeSet = false;

		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-channel object found in animation channels");
			continue;
		}

		/*
		Animation channels can contain the following values:
		sampler:		The index of the sampler for the animation channel.
		target::node:	The target node affected by this channel.
		target::path:	The part of the model affected by the channel.
		*/
		for (const auto&[key, val] : cur.items())
		{
			string upperKey = string(key).toUpper();
			if (upperKey == "SAMPLER")
			{
				if (val.is_number_unsigned()) channel.Sampler = val;
				else LogCorruptJsonHeader("animation channel sampler isn't a valid index");
			}
			else if (upperKey == "TARGET")
			{
				if (!val.is_object())
				{
					LogCorruptJsonHeader("animation channel targets isn't an object");
					continue;
				}

				for (const auto &[targetKey, targetVal] : val.items())
				{
					upperKey = string(targetKey).toUpper();
					if (upperKey == "NODE")
					{
						if (targetVal.is_number_unsigned())
						{
							nodeSet = true;
							channel.TargetNode = targetVal;
						}
						else LogCorruptJsonHeader("animation channel target node isn't a valid index");
					}
					else if (upperKey == "PATH")
					{
						if (targetVal.is_string())
						{
							const string path = string(targetVal).toUpper();
							if (path == "TRANSLATION") channel.Target = GLTFAnimationTarget::Translation;
							else if (path == "ROTATION") channel.Target = GLTFAnimationTarget::Rotation;
							else if (path == "SCALE") channel.Target = GLTFAnimationTarget::Scale;
							else if (path == "WEIGHTS") channel.Target = GLTFAnimationTarget::Weights;
							else LogCorruptJsonHeader("animation channel target path invalid");
						}
						else LogCorruptJsonHeader("animation channel path isn't a string");
					}
				}
			}
		}

		/* Only add the channel if the node is set, otherwise it's useless. */
		if (nodeSet) animation.Channels.emplace_back(channel);
	}
}

void HandleJsonAnimationSamplers(const json &samplers, GLTFAnimation &animation)
{
	CHECK_IF_ARRAY(samplers);

	animation.Samplers.reserve(samplers.size());
	for (const json &cur : samplers)
	{
		GLTFAnimationSampler sampler;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-sampler object found in animation samplers");
			continue;
		}

		/*
		Animation samplers can contain the following values:
		input:			The index of the sampler accessor timestamps
		output:			The index of the sampler accessor frames
		interpolation:	The type of inter-frame interpolation
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "INPUT")
			{
				if (val.is_number_unsigned()) sampler.Input = val;
				else LogCorruptJsonHeader("animation sampler input isn't a valid index");
			}
			else if (upperKey == "OUTPUT")
			{
				if (val.is_number_unsigned()) sampler.Output = val;
				else LogCorruptJsonHeader("animation sampler output isn't a valid index");
			}
			else if (upperKey == "INTERPOLATION")
			{
				if (val.is_string())
				{
					const string interpolation = string(val).toUpper();
					if (interpolation == "STEP") sampler.Interpolation = GLTFInterpolation::Step;
					else if (interpolation == "LINEAR") sampler.Interpolation = GLTFInterpolation::Linear;
					else if (interpolation == "CUBICSPLINE") sampler.Interpolation = GLTFInterpolation::Cubic;
					else LogCorruptJsonHeader("invalid animation sampler interpolation");
				}
				else LogCorruptJsonHeader("animation sampler interpolation isn't a string");
			}
		}

		animation.Samplers.emplace_back(sampler);
	}
}

void HandleJsonAnimations(const json &animations, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(animations);

	/* Loops over the meshes in the array */
	file.Animations.reserve(animations.size());
	for (const json &cur : animations)
	{
		/* Animations must always be objects. */
		GLTFAnimation animation;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-animation object found in animations");
			continue;
		}

		/*
		An animation can contain the following values:
		channels:	Defines which parts of the model should be animated
		samplers:	Defines how specific frames should be handled
		name:		Gives an optional name to the animation
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "NAME")
			{
				if (val.is_string()) animation.Name = val;
				else LogCorruptJsonHeader("animation name isn't a string");
			}
			else if (upperKey == "CHANNELS") HandleJsonAnimationChannels(val, animation);
			else if (upperKey == "SAMPLERS") HandleJsonAnimationSamplers(val, animation);
		}

		file.Animations.emplace_back(animation);
	}
}

void HandleJsonSkins(const json &skins, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(skins);

	/* Loop over all skins in the array. */
	file.Skins.reserve(skins.size());
	for (const json &cur : skins)
	{
		/* Skins must always be objects. */
		GLTFSkin skin;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-skin object found in skins");
			continue;
		}

		/*
		Skins contain the following fields:
		name:					An optional name for the skin.
		skeleton:				An optional common root node for the joints.
		inverseBindMatrices:	The index to the IBM's accessor.
		joints:					The nodes affected by the skin.
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "NAME")
			{
				if (val.is_string()) skin.Name = val;
				else LogCorruptJsonHeader("skin name isn't a string");
			}
			else if (upperKey == "SKELETON")
			{
				if (val.is_number_unsigned()) skin.Skeleton = val;
				else LogCorruptJsonHeader("skin skeleton isn't a valid index");
			}
			else if (upperKey == "INVERSEBINDMATRICES")
			{
				if (val.is_number_unsigned()) skin.IBindMatrices = val;
				else LogCorruptJsonHeader("skin IBM isn't a valid index");
			}
			else if (upperKey == "JOINTS")
			{
				if (!val.is_array())
				{
					LogCorruptJsonHeader("skin joints aren't in an array");
					continue;
				}

				/* Loop over all joint indeces and add them. */
				skin.Joints.reserve(val.size());
				for (const json &idx : val)
				{
					if (idx.is_number_unsigned()) skin.Joints.emplace_back(idx);
					else LogCorruptJsonHeader("skin joint isn't a valid index");
				}
			}
		}

		file.Skins.emplace_back(skin);
	}
}

void HandleJsonAccessors(const json &accessors, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(accessors);

	/* Loop over every accessor in the array. */
	file.Accessors.reserve(accessors.size());
	for (const json &cur : accessors)
	{
		/* Accessors are always defined as objects. */
		GLTFAccessor accessor;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-accessor object found in accessors");
			continue;
		}

		/*
		An accessor can contain the following values:
		bufferView:			The index to the associated buffer view.
		byteOffset:			The offset to start reading the the view.
		count:				The amount of elements accessible.
		type:				The element type.
		componentType:		The underlying type.
		max:				An optional maximum value.
		min:				An optional minimum value.
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "BUFFERVIEW")
			{
				if (val.is_number_unsigned()) accessor.BufferView = val;
				else LogCorruptJsonHeader("accessor bufferview isn't a valid index");
			}
			else if (upperKey == "BYTEOFFSET")
			{
				if (val.is_number_unsigned()) accessor.Start = val;
				else LogCorruptJsonHeader("accessor byte offset isn't valid");
			}
			else if (upperKey == "COUNT")
			{
				if (val.is_number_unsigned()) accessor.Count = val;
				else LogCorruptJsonHeader("accessor count isn't valid");
			}
			else if (upperKey == "TYPE")
			{
				if (val.is_string())
				{
					/* Convert from string to enum for faster checking later (SCALAR needs no checking because it's the default). */
					const string type = string(val).toUpper();
					if (type == "SCALAR") accessor.Type = GlTfType::Scalar;
					else if (type == "VEC2") accessor.Type = GlTfType::Vector2;
					else if (type == "VEC3") accessor.Type = GlTfType::Vector3;
					else if (type == "VEC4") accessor.Type = GlTfType::Vector4;
					else if (type == "MAT2") accessor.Type = GlTfType::Matrix2;
					else if (type == "MAT3") accessor.Type = GlTfType::Matrix3;
					else if (type == "MAT4") accessor.Type = GlTfType::Matrix4;
				}
				else LogCorruptJsonHeader("accessor type isn't a string");
			}
			else if (upperKey == "COMPONENTTYPE")
			{
				if (val.is_number_unsigned()) accessor.ComponentType = static_cast<GlTfComponentType>(val);
				else LogCorruptJsonHeader("accessor component type isn't valid");
			}
			else if (upperKey == "MAX")
			{
				if (val.is_array())
				{
					accessor.Maximum.reserve(val.size());
					for (const json &maxVal : val)
					{
						if (maxVal.is_number()) accessor.Maximum.emplace_back(maxVal);
						else LogCorruptJsonHeader("accessor maximum isn't a valid number");
					}
				}
				else LogCorruptJsonHeader("accessor maximum isn't an array");
			}
			else if (upperKey == "MIN")
			{
				if (val.is_array())
				{
					accessor.Minimum.reserve(val.size());
					for (const json &minVal : val)
					{
						if (minVal.is_number()) accessor.Minimum.emplace_back(minVal);
						else LogCorruptJsonHeader("accessor minimum isn't a valid number");
					}
				}
				else LogCorruptJsonHeader("accessor minimum isn't an array");
			}
		}

		file.Accessors.emplace_back(accessor);
	}
}

void HandleJsonMaterialNestedObject(const json &object, GLTFMaterial &material, GLTFParameter *parent)
{
	/* Loop over the values in the object. */
	for (const auto&[key, val] : object.items())
	{
		const string upperKey = string(key).toUpper();
		GLTFParameter param;
		bool shouldAdd = false;

		/* Booleans, strings, numbers and arrays are handled directly. */
		if (val.is_boolean())
		{
			shouldAdd = true;
			param.BooleanValue = val;
		}
		else if (val.is_string())
		{
			shouldAdd = true;
			param.StringValue = val;
		}
		else if (val.is_number())
		{
			/* If this value is nested, add it to the named numbers of its parent, otherwise; just add it. */
			if (parent)
			{
				parent->NamedNumbers.emplace(upperKey, val);
				continue;
			}
			else
			{
				shouldAdd = true;
				param.HasNumberValue = true;
				param.NumberValue = val;
			}
		}
		else if (val.is_array())
		{
			/* If these values are nested, add them to the named numbers of its parents, otherwise; add them to their own numbers. */
			if (parent)
			{
				for (const json &number : val)
				{
					if (number.is_number()) parent->NamedNumbers.emplace(upperKey, number);
					else Log::Warning("GLTF loaded doesn't currently handle non-number arrays in materials!");
				}

				continue;
			}
			else
			{
				shouldAdd = true;

				for (const json &number : val)
				{
					if (number.is_number()) param.Numbers.emplace_back(number);
					else Log::Warning("GLTF loader doesn't currently handle non-number arrays in materials!");
				}
			}
		}
		else if (val.is_object())
		{
			HandleJsonMaterialNestedObject(val, material, &param);
			shouldAdd = param.HasNumberValue || param.Numbers.size() > 0 || param.NamedNumbers.size() > 0;
		}

		/* By default add the value object to the material. */
		if (shouldAdd) material.Values.emplace(upperKey, param);
	}
}

void HandleJsonMaterials(const json &materials, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(materials);

	/* Loop through all materials in the array. */
	file.Materials.reserve(materials.size());
	for (const json &cur : materials)
	{
		/* A material should always be an object. */
		GLTFMaterial material;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-material object found in materials");
			continue;
		}

		/* Handle the current material properties. */
		HandleJsonMaterialNestedObject(cur, material, nullptr);

		/* Remove the name property and hard set it into the material if it's present. */
		for (auto &[key, val] : material.Values)
		{
			if (key == "NAME")
			{
				material.Name = std::move(val.StringValue);
				material.Values.erase("NAME");
				break;
			}
		}

		file.Materials.emplace_back(material);
	}
}

void HandleJsonTextures(const json &textures, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(textures);

	/* Loop over all tetxures in the array. */
	file.Textures.reserve(textures.size());
	for (const json &cur : textures)
	{
		/* A texture must be an object. */
		GLTFTexture texture;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-texture object found in textures");
			continue;
		}

		/* Textures contain only a sampler and image source index. */
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "SAMPLER")
			{
				if (val.is_number_unsigned()) texture.Sampler = val;
				else LogCorruptJsonHeader("texture sampler isn't a valid index");
			}
			else if (upperKey == "SOURCE")
			{
				if (val.is_number_unsigned()) texture.Image = val;
				else LogCorruptJsonHeader("texture image source ins't a valid index");
			}
		}

		file.Textures.emplace_back(texture);
	}
}

void HandleJsonImages(const json &images, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(images);

	/* Loop through all images in the array. */
	file.Images.reserve(images.size());
	for (const json &cur : images)
	{
		/* Images must be objects. */
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-image object found in images");
			continue;
		}

		/*
		An image can be defined in three ways:
		uri (text):					A path to an image file.
		uri (base64-encoded):		embedded data (not handled)
		bufferview / mineType:		a reference to a bufferview (not handled)
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string KEY = string(key).toUpper();

			if (KEY == "URI")
			{
				if (val.is_string()) file.Images.emplace_back(string(val).toWide());
				else LogCorruptJsonHeader("Image URI isn't a valid path");
			}
			else if (KEY != "MIMETYPE") Log::Warning("GLTF loader doesn't currently handle embedded images!");
		}
	}
}

void HandleJsonImageSamplers(const json &samplers, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(samplers);

	/* Loop through all samplers in the array. */
	file.Samplers.reserve(samplers.size());
	for (const json &cur : samplers)
	{
		/* Samplers must be objects. */
		GLTFImageSampler sampler;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-image object found in images");
			continue;
		}

		/*
		A sampler defines the following fields:
		magFilter:		The magnification filter.
		minFilter:		The minification filter.
		wrapS:			The texture wrapping on the S axis.
		wrapT:			The texture wrapping on the T axis.
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "MAGFILTER")
			{
				if (val.is_number_unsigned()) sampler.MagFilter = static_cast<GLTFFilter>(val);
				else LogCorruptJsonHeader("sampler magnification filter isn't valid");
			}
			else if (upperKey == "MINFILTER")
			{
				if (val.is_number_unsigned()) sampler.MinFilter = static_cast<GLTFFilter>(val);
				else LogCorruptJsonHeader("sampler minification filter isn't valid");
			}
			else if (upperKey == "WRAPS")
			{
				if (val.is_number_unsigned()) sampler.WrapS = static_cast<GLTFWrap>(val);
				else LogCorruptJsonHeader("sampler wrap S isn't valid");
			}
			else if (upperKey == "WRAPT")
			{
				if (val.is_number_unsigned()) sampler.WrapT = static_cast<GLTFWrap>(val);
				else LogCorruptJsonHeader("sampler wrap T isn't valid");
			}
		}

		file.Samplers.emplace_back(sampler);
	}
}

void HandleJsonBufferViews(const json &bufferViews, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(bufferViews);

	/* Loop through all buffer views in the array. */
	file.BufferViews.reserve(bufferViews.size());
	for (const json &cur : bufferViews)
	{
		/* Buffer views must be objects. */
		GLTFBufferView bufferView;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-buffer view object found in buffer views");
			continue;
		}

		/*
		A buffer view defines the following fields:
		buffer:			The associated buffer index.
		byteLength:		The amount of byte.
		byteOffset:		The start of the data.
		target:			The OpenGL target (not used)
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "BUFFER")
			{
				if (val.is_number_unsigned()) bufferView.Buffer = val;
				else LogCorruptJsonHeader("buffer view buffer isn't a valid index");
			}
			else if (upperKey == "BYTELENGTH")
			{
				if (val.is_number_unsigned()) bufferView.Length = val;
				else LogCorruptJsonHeader("buffer view length isn't valid");
			}
			else if (upperKey == "BYTEOFFSET")
			{
				if (val.is_number_unsigned()) bufferView.Start = val;
				else LogCorruptJsonHeader("buffer view offset isn't valid");
			}
		}

		file.BufferViews.emplace_back(bufferView);
	}
}

void HandleJsonBuffers(const json &buffers, GLTFLoaderResult &file)
{
	CHECK_IF_ARRAY(buffers);

	/* Loop through all bufferss in the array. */
	file.Buffers.reserve(buffers.size());
	for (const json &cur : buffers)
	{
		/* Buffers must be objects. */
		GLTFBuffer buffer;
		if (!cur.is_object())
		{
			LogCorruptJsonHeader("non-buffer object found in buffers");
			continue;
		}

		/*
		A buffer defines the following fields:
		uri:			The path to the buffers .bin file.
		byteLength:		The length (in bytes) of the buffer.
		*/
		for (const auto&[key, val] : cur.items())
		{
			const string upperKey = string(key).toUpper();
			if (upperKey == "URI")
			{
				if (val.is_string()) buffer.Uri = string(val).toWide();
				else LogCorruptJsonHeader("buffer URI isn't a string");
			}
			else if (upperKey == "BYTELENGTH")
			{
				if (val.is_number_unsigned()) buffer.Size = val;
				else LogCorruptJsonHeader("buffer size isn't valid");
			}
		}

		file.Buffers.emplace_back(buffer);
	}
}

void LoadJsonGLTF(const string &source, GLTFLoaderResult &file)
{
	/* Parse the source string into a parsable format. */
	const json header = json::parse(source);

	/* Make sure the first node is valid. */
	if (!header.is_object())
	{
		Log::Error("Corrupt GLTF Json header (first node isn't an object)!");
		return;
	}

	/* Loop over the childs and handle them. */
	for (const auto &[key, val] : header.items())
	{
		const string elementType = string(key).toUpper();
		if (elementType == "ASSET")
		{
			/* Checks whether the asset can be parsed by this parser so return is it return true. */
			if (HandleJsonAsset(val)) return;
		}
		else if (elementType == "SCENE") HandleJsonDefaultScene(val, file);
		else if (elementType == "SCENES") HandleJsonScenes(val, file);
		else if (elementType == "NODES") HandleJsonNodes(val, file);
		else if (elementType == "MESHES") HandleJsonMeshes(val, file);
		else if (elementType == "ANIMATIONS") HandleJsonAnimations(val, file);
		else if (elementType == "SKINS") HandleJsonSkins(val, file);
		else if (elementType == "ACCESSORS")  HandleJsonAccessors(val, file);
		else if (elementType == "MATERIALS") HandleJsonMaterials(val, file);
		else if (elementType == "TEXTURES") HandleJsonTextures(val, file);
		else if (elementType == "IMAGES") HandleJsonImages(val, file);
		else if (elementType == "SAMPLERS") HandleJsonImageSamplers(val, file);
		else if (elementType == "BUFFERVIEWS") HandleJsonBufferViews(val, file);
		else if (elementType == "BUFFERS") HandleJsonBuffers(val, file);
		else
		{
			/* Just log a warning for all unhandled types. */
			Log::Warning("Unable to handle element type '%s' in GLTF header at this point!", elementType.c_str());
		}
	}
}

void LoadGLTF(const CLArgs & args, GLTFLoaderResult & file)
{
	/* Make sure the file is valid. */
	const string ext = args.Input.fileExtension().toUpper();
	const wstring dir = args.Input.fileDirectory().toWide();

	if (ext == "GLTF")
	{
		/* Only load the header of the GLTF from Json format. */
		FileReader reader(args.Input.toWide());
		LoadJsonGLTF(reader.ReadToEnd(), file);
	}
	else if (ext == "GLB")
	{
		/* Should implement this for faster loading at some point. */
		Log::Error("Unable to load binary GLTF at this time!");
	}
	else Log::Error("Attempting to load .%s file '%s' as GLTF file!", ext.c_str(), args.Input.fileNameWithoutExtension().c_str());

	/* Add the file's directory to all buffers and images. */
	for (GLTFBuffer &cur : file.Buffers) cur.Uri = dir + cur.Uri;
	for (wstring &cur : file.Images) cur = dir + cur;
}

void CopyNodesToPum(const GLTFLoaderResult &input, PumIntermediate &result)
{
	/* Just copy over the nodes. */
	result.Nodes.reserve(input.Nodes.size());
	for (const GLTFNode &cur : input.Nodes)
	{
		pum_node node;
		node.Children = cur.Children.select<uint32>([](size_t in) {return static_cast<uint32>(in); });

		if (cur.HasMesh) node.SetMesh(static_cast<uint32>(cur.Mesh));
		if (cur.HasSkin) node.SetSkin(static_cast<uint32>(cur.Skin));
		if (cur.Translation != Vector3()) node.SetTranslation(cur.Translation);
		if (cur.Rotation != Quaternion()) node.SetRotation(cur.Rotation);
		if (cur.Scale != Vector3()) node.SetScale(cur.Scale);

		result.Nodes.emplace_back(node);
	}
}

size_t CopyMeshAttribute(const vector<string> &bufferData, byte *destination, size_t offset, size_t stride, const GLTFLoaderResult &input, const GLTFPrimitive &primitive, GLTFPrimitiveAttribute type)
{
	/* Check if the primitive attribute is present. */
	size_t idx;
	if (primitive.TryGetAttribute(type, idx))
	{
		const GLTFAccessor &accessor = input.Accessors[idx];
		const GLTFBufferView &view = input.BufferViews[accessor.BufferView];
		const size_t attribSize = accessor.GetElementSize();
		const char *data = bufferData[view.Buffer].data();

		/* Copy the data. */
		for (size_t i = 0, j = view.Start + accessor.Start; i < accessor.Count; i++, j += attribSize, offset += stride)
		{
			memcpy(destination + offset, data + j, attribSize);
		}

		return attribSize;
	}

	return 0;
}

void CopyMeshesToPum(const GLTFLoaderResult &input, const vector<string> &bufferData, PumIntermediate &result)
{
	/* A GLTF mesh is not the same as a Plutonium mesh so we don't do much with it. */
	for (const GLTFMesh &glMesh : input.Meshes)
	{
		Log::Verbose("Processing mesh: %s.", glMesh.Name.c_str());

		/* A GLTF primitive is a Plutonium mesh so start from here. */
		for (const GLTFPrimitive &cur : glMesh.Primitives)
		{
			/* Copy over the easy data. */
			pum_mesh mesh;
			mesh.Identifier = glMesh.Name.toUTF32();
			if (cur.HasMaterial) mesh.SetMaterial(static_cast<uint32>(cur.Material));

			/* Set the mode (sadly this cannot be a cast). */
			switch (cur.Mode)
			{
			case GLTFMode::Points:
				mesh.Topology = 0;
				break;
			case GLTFMode::Line:
				mesh.Topology = 1;
				break;
			case GLTFMode::LineLoop:
				mesh.Topology = 2;
				break;
			case GLTFMode::Triangles:
				mesh.Topology = 3;
				break;
			case GLTFMode::TriangleStrip:
				mesh.Topology = 4;
				break;
			case GLTFMode::TriangleFan:
				mesh.Topology = 5;
				break;
			}

			/* Handle indices if they're present. */
			if (cur.HasIndices)
			{
				const GLTFAccessor &accessor = input.Accessors[cur.Indices];
				const GLTFBufferView &view = input.BufferViews[accessor.BufferView];

				/* Set the index mode and do some error checking. */
				if (accessor.Type != GlTfType::Scalar)
				{
					Log::Error("Primitive in mesh '%s' has an invalid index accessor type!", glMesh.Name.c_str());
					continue;
				}
				else if (accessor.ComponentType == GlTfComponentType::UInt16) mesh.IndexMode = 0;
				else if (accessor.ComponentType == GlTfComponentType::UInt32) mesh.IndexMode = 1;
				else
				{
					Log::Error("Primitive in mesh '%s' has an invalid index accessor component type!", glMesh.Name.c_str());
					continue;
				}

				/* Copy over the index data to our buffer and fill in the data for the result mesh. */
				mesh.IndexViewStart = result.Data.GetSize();
				mesh.IndexViewSize = view.Length - accessor.Start;
				result.Data.Write(reinterpret_cast<const byte*>(bufferData[view.Buffer].data()), view.Start + accessor.Start, mesh.IndexViewSize);
			}

			/* Calculate the size (in bytes) of the vertex buffer (for this mesh) and its stride. */
			size_t vrtxStrideBytes = 0;
			mesh.VertexViewSize = 0;
			for (const auto[type, idx] : cur.Attributes)
			{
				const GLTFAccessor &accessor = input.Accessors[idx];
				const GLTFBufferView &view = input.BufferViews[accessor.BufferView];

				mesh.VertexViewSize += view.Length - accessor.Start;
				vrtxStrideBytes += accessor.GetElementSize();

				switch (type)
				{
				case GLTFPrimitiveAttribute::Position:
					mesh.Bounds.LowerBound.X = static_cast<float>(accessor.Minimum[0]);
					mesh.Bounds.LowerBound.Y = static_cast<float>(accessor.Minimum[1]);
					if (accessor.Minimum.size() > 2) mesh.Bounds.LowerBound.Z = static_cast<float>(accessor.Minimum[2]);

					mesh.Bounds.UpperBound.X = static_cast<float>(accessor.Maximum[0]);
					mesh.Bounds.UpperBound.Y = static_cast<float>(accessor.Maximum[1]);
					if (accessor.Maximum.size() > 2) mesh.Bounds.UpperBound.Z = static_cast<float>(accessor.Maximum[2]);
					break;
				case GLTFPrimitiveAttribute::Normal:
					mesh.HasNormals = true;
					break;
				case GLTFPrimitiveAttribute::Tangent:
					mesh.HasTangents = true;
					break;
				case GLTFPrimitiveAttribute::TexCoord1:
					mesh.HasTextureUvs = true;
					break;
				case GLTFPrimitiveAttribute::TexCoord2:
					Log::Warning("Plutonium models cannot have more than 1 set of texture coordiantes!");
					break;
				case GLTFPrimitiveAttribute::Color:
					mesh.HasVertexColors = true;
					break;
				case GLTFPrimitiveAttribute::Joints:
					if (accessor.ComponentType == GlTfComponentType::UInt8) mesh.HasJoints = 1;
					else if (accessor.ComponentType == GlTfComponentType::UInt16) mesh.HasJoints = 2;
					break;
				}
			}

			/* Copy the actual vertex data to the output buffer. */
			mesh.VertexViewStart = result.Data.GetSize();
			byte *tmp = reinterpret_cast<byte*>(malloc(mesh.VertexViewSize));
			size_t offset = 0;

			/* Make sure that the mesh starts at a alligned offset. */
			const size_t zeroBytes = mesh.VertexViewStart % vrtxStrideBytes;
			result.Data.Pad(zeroBytes);
			mesh.VertexViewStart += zeroBytes;

			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::Position);
			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::Normal);
			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::Tangent);
			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::TexCoord1);
			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::Color);
			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::Joints);
			offset += CopyMeshAttribute(bufferData, tmp, offset, vrtxStrideBytes, input, cur, GLTFPrimitiveAttribute::Weights);

			result.Data.Write(tmp, 0, mesh.VertexViewSize);
			free(tmp);

			result.Geometry.emplace_back(mesh);
		}
	}
}

void ProcessAnimationFrame(BinaryReader &reader, const GLTFAccessor &accessor, GLTFAnimationTarget target, pum_frame &result)
{
	if (target == GLTFAnimationTarget::Translation) result.Translation = reader.ReadVector3();
	else if (target == GLTFAnimationTarget::Scale) result.Scale = reader.ReadVector3();
	else if (target == GLTFAnimationTarget::Rotation)
	{
		float i, j, k, r;

		if (accessor.ComponentType == GlTfComponentType::Float)
		{
			result.Rotation = reader.ReadQuaternion();
			return;
		}
		else if (accessor.ComponentType == GlTfComponentType::Int8)
		{
			i = max(reader.ReadSByte() / 127.0f, -1.0f);
			j = max(reader.ReadSByte() / 127.0f, -1.0f);
			k = max(reader.ReadSByte() / 127.0f, -1.0f);
			r = max(reader.ReadSByte() / 127.0f, -1.0f);
		}
		else if (accessor.ComponentType == GlTfComponentType::UInt8)
		{
			i = reader.ReadByte() / 255.0f;
			j = reader.ReadByte() / 255.0f;
			k = reader.ReadByte() / 255.0f;
			r = reader.ReadByte() / 255.0f;
		}
		else if (accessor.ComponentType == GlTfComponentType::Int16)
		{
			i = max(reader.ReadInt16() / 32767.0f, -1.0f);
			j = max(reader.ReadInt16() / 32767.0f, -1.0f);
			k = max(reader.ReadInt16() / 32767.0f, -1.0f);
			r = max(reader.ReadInt16() / 32767.0f, -1.0f);
		}
		else if (accessor.ComponentType == GlTfComponentType::UInt16)
		{
			i = reader.ReadUInt16() / 65535.0f;
			j = reader.ReadUInt16() / 65535.0f;
			k = reader.ReadUInt16() / 65535.0f;
			r = reader.ReadUInt16() / 65535.0f;
		}

		result.Rotation = Quaternion(r, i, j, k);
	}
}

void ProcessAnimationChannel(const GLTFLoaderResult &input, const vector<string> &bufferData, const GLTFChannel &channel, const GLTFAnimationSampler &sampler, pum_sequency &result)
{
	/* Get the input and output accessors and views. */
	const GLTFAccessor &inputAccessor = input.Accessors[sampler.Input];
	const GLTFAccessor &outputAccessor = input.Accessors[sampler.Output];
	const GLTFBufferView &inputView = input.BufferViews[inputAccessor.BufferView];
	const GLTFBufferView &outputView = input.BufferViews[outputAccessor.BufferView];

	/* Check for corrupt files. */
	if (inputAccessor.Count != outputAccessor.Count)
	{
		Log::Error("Corrupt GLTF animation sampler detected, input and output sizes differ!");
		return;
	}

	if (inputAccessor.Type != GlTfType::Scalar || inputAccessor.ComponentType != GlTfComponentType::Float)
	{
		Log::Error("Corrupt GLTF animation sampler detected, input accessor doesn't define seconds as floats!");
		return;
	}

	if (channel.Target == GLTFAnimationTarget::Translation && (outputAccessor.Type != GlTfType::Vector3 || outputAccessor.ComponentType != GlTfComponentType::Float))
	{
		Log::Error("Corrupt GLTF animation channel detected, translation channels must be float vec3!");
		return;
	}

	if (channel.Target == GLTFAnimationTarget::Scale && (outputAccessor.Type != GlTfType::Vector3 || outputAccessor.ComponentType != GlTfComponentType::Float))
	{
		Log::Error("Corrupt GLTF animation channel detected, scale channels must be float vec3!");
		return;
	}

	if (channel.Target == GLTFAnimationTarget::Weights)
	{
		Log::Error("Plutonium currently doesn't handle weighted morph animation channels!");
		return;
	}

	if (channel.Target == GLTFAnimationTarget::Rotation && (outputAccessor.Type != GlTfType::Vector4 ||
		(outputAccessor.ComponentType != GlTfComponentType::Float &&
		outputAccessor.ComponentType != GlTfComponentType::Int8 &&
		outputAccessor.ComponentType != GlTfComponentType::UInt8 &&
		outputAccessor.ComponentType != GlTfComponentType::Int16 &&
		outputAccessor.ComponentType != GlTfComponentType::UInt16)))
	{
		Log::Error("Corrupt GLTF animation channel detected, rotation channel doesn't have a valid type or component type!");
		return;
	}

	BinaryReader inputReader(bufferData[inputView.Buffer].data() + inputView.Start + inputAccessor.Start, inputView.Length - inputAccessor.Start);
	BinaryReader outputReader(bufferData[outputView.Buffer].data() + outputView.Start + outputAccessor.Start, outputView.Length - outputAccessor.Start);

	/* Process the frames. */
	for (size_t i = 0; i < inputAccessor.Count; i++)
	{
		const float timestamp = inputReader.ReadSingle();

		/* Check if the timestamp for the frame is already defined, if so just add it to the frame. */
		vector<pum_frame>::iterator it = result.Frames.iteratorOf([timestamp](const pum_frame &cur) { return cur.Time == timestamp; });
		if (it == result.Frames.end())
		{
			pum_frame frame;
			frame.Time = timestamp;
			ProcessAnimationFrame(outputReader, outputAccessor, channel.Target, frame);
			result.Frames.emplace_back(frame);
		}
		else ProcessAnimationFrame(outputReader, outputAccessor, channel.Target, *it);
	}
}

void CopyAnimationsToPum(const GLTFLoaderResult &input, const vector<string> &bufferData, PumIntermediate &result)
{
	result.Animations.reserve(input.Animations.size());
	for (const GLTFAnimation &cur : input.Animations)
	{
		/* Set the name. */
		pum_animation anim;
		anim.Identifier = cur.Name.toUTF32();
		anim.InterpolationMode = -1;

		/* Set the interpolation mode. */
		for (const GLTFAnimationSampler &sampler : cur.Samplers)
		{
			const int newInterpolationMode = static_cast<int>(sampler.Interpolation);

			if (anim.InterpolationMode == -1)
			{
				anim.InterpolationMode = newInterpolationMode;
				if (sampler.Interpolation == GLTFInterpolation::Cubic)
				{	//TODO: change this to proper values.
					anim.Arg1 = 0.0f;
					anim.Arg2 = 0.0f;
				}
			}
			else if (anim.InterpolationMode != newInterpolationMode)
			{
				Log::Warning("Multiple interpolation mode are defined in animation '%s' newer ones are ignored!", cur.Name.c_str());
			}
		}

		/* Convert the channels into sequences. */
		anim.Sequences.reserve(cur.Channels.size());
		for (const GLTFChannel &channel : cur.Channels)
		{
			/* Check if we need to add a new sequence or if the node is already present. */
			vector<pum_sequency>::iterator it = anim.Sequences.iteratorOf([&channel](const pum_sequency &seq) { return seq.Node == channel.TargetNode; });
			if (it == anim.Sequences.end())
			{
				pum_sequency seq;
				seq.Node = static_cast<uint32>(channel.TargetNode);
				ProcessAnimationChannel(input, bufferData, channel, cur.Samplers[channel.Sampler], seq);
				anim.Sequences.emplace_back(seq);
			}
			else ProcessAnimationChannel(input, bufferData, channel, cur.Samplers[channel.Sampler], *it);
		}

		result.Animations.emplace_back(anim);
	}
}

void CopySkeletonToPum(const GLTFLoaderResult &input, const vector<string> &bufferData, PumIntermediate &result)
{
	result.Skeletons.reserve(input.Skins.size());
	for (const GLTFSkin &skin : input.Skins)
	{
		/* Set the name and the root node. */
		pum_skeleton skeleton;
		skeleton.Identifier = skin.Name.toUTF32();
		skeleton.Root = static_cast<uint32>(skin.Skeleton);

		/* Get the start of the inverse bind matrices data. */
		const GLTFAccessor &accessor = input.Accessors[skin.IBindMatrices];
		const GLTFBufferView &view = input.BufferViews[accessor.BufferView];
		BinaryReader reader(bufferData[view.Buffer].data() + view.Start + accessor.Start, view.Length - accessor.Start);

		/* Check for errors. */
		if (accessor.Type != GlTfType::Matrix4 || accessor.ComponentType != GlTfComponentType::Float)
		{
			Log::Error("Corrupted GLTF skin detected, inverse bind matrices aren't 4x4 float matrices in skin '%s'!", skin.Name.c_str());
			continue;
		}

		/* Copy the underlying joints over. */
		skeleton.Joints.reserve(skin.Joints.size());
		for (size_t cur : skin.Joints)
		{
			pum_joint joint;
			joint.Node = static_cast<uint32>(cur);
			joint.IBind = reader.ReadMatrix();
			skeleton.Joints.emplace_back(joint);
		}

		result.Skeletons.emplace_back(skeleton);
	}
}

void CopyMaterialsToPum(const GLTFLoaderResult &input, PumIntermediate &result)
{
	result.Materials.reserve(input.Materials.size());
	for (const GLTFMaterial &cur : input.Materials)
	{
		pum_material material;
		material.Identifier = cur.Name.toUTF32();
		material.SpecularPower = 2.0f; // GLTF uses GGX Trowbridge-Reitz for microfacets, so the specular power should be 2 to match that curve.

		/* Check for the double sided flag. */
		GLTFParameter value;
		if (cur.TryGetValue("DOUBLESIDED", value)) material.DoubleSided = value.BooleanValue;

		/* Set the alpha mode (opaque is the default). */
		if (cur.TryGetValue("ALPHAMODE", value))
		{
			if (value.StringValue == "MASK")
			{
				material.AlphaMode = 1;

				/* GLTF defines a default alpha cutoff as 0.5. */
				if (cur.TryGetValue("ALPHACUTOFF", value)) material.AlphaThreshold = static_cast<float>(value.NumberValue);
				else material.AlphaThreshold = 0.5f;
			}
			else if (value.StringValue == "BLEND") material.AlphaMode = 2;
		}

		/* Check for the emissive factor and intensity. */
		if (cur.TryGetValue("EMISSIVEFACTOR", value)) material.EmissiveFactor = value.GetHDRColor(material.EmissiveInternsity);

		/* Check for diffuse or albedo texture. */
		if (cur.TryGetValue("DIFFUSETEXTURE", value)) material.SetDiffuseTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));
		else if (cur.TryGetValue("BASECOLORTEXTURE", value))
		{
			material.IsFinalized = false;	// We need to convert the albedo texture.
			material.SetDiffuseTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));
			result.Textures[material.DiffuseTexture].ConversionCount = 0; // Make sure that the texture gets converted.
		}

		/* Check for specular glossiness texture. */
		if (cur.TryGetValue("SPECULARGLOSSINESSTEXTURE", value)) material.SetSpecGlossTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));
		else if (cur.TryGetValue("METALLICROUGHNESSTEXTURE", value))
		{
			material.IsFinalized = false;	// We need to convert the metal roughness texture.
			material.SetSpecGlossTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));
			result.Textures[material.SpecGlossTexture].ConversionCount = 0;	// Make sure that the texture gets converted.
		}

		/*
		Check for the diffuse factor or the albedo factor.
		This is used as a scalar if a map is defined so de default changed if a map is defined.
		*/
		Color albedo = Color::Black();
		if (cur.TryGetValue("DIFFUSEFACTOR", value)) material.DiffuseFactor = value.GetColor();
		else if (cur.TryGetValue("BASECOLORFACTOR", value)) albedo = value.GetColor();
		else material.DiffuseFactor = Color::White(); // GLTF states that the default should be 1.

		/* Check for the specular factor. */
		if (cur.TryGetValue("SPECULARFACTOR", value)) material.SpecularFactor = value.GetColor();
		else material.SpecularFactor = Color::White(); // GLTF states that the default should be 1.

		/* This is a way for the material to set global scalars, it tends to only be used for objects with no textures. */
		if (cur.TryGetValue("PBRMETALLICROUGHNESS", value))
		{
			double number;
			if (value.TryGetNamedNumber("METALLICFACTOR", number))
			{
				material.Metalness = static_cast<float>(number);	// We save the metalness for during the conversion stage.

				/* The diffuse and specular color are only set when metalness is defined for the entire object. */
				material.DiffuseFactor = Color::Lerp(albedo, Color::Black(), material.Metalness);
				material.SpecularFactor = Color::Lerp(Color::CodGray(), material.DiffuseFactor, material.Metalness);
			}

			if (value.TryGetNamedNumber("ROUGHNESSFACTOR", number)) material.Glossiness = 1.0f - static_cast<float>(number);
		}

		/* Check for glossiness or roughness. */
		if (cur.TryGetValue("GLOSSINESSFACTOR", value)) material.Glossiness = static_cast<float>(value.NumberValue);
		else if (material.IsFinalized) material.Glossiness = 1.0f;	// GLTF states that glossiness should have a default value of 1 (if specular gloss is used!), Plutonium defaults to 0.

		/* Check for normal texture. */
		if (cur.TryGetValue("NORMALTEXTURE", value)) material.SetNormalTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));

		/* Check for ambient occlusion texture. */
		if (cur.TryGetValue("OCCLUSIONTEXTURE", value)) material.SetOcclusionTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));

		/* Check for ambient occlusion texture. */
		if (cur.TryGetValue("EMISSIVETEXTURE", value)) material.SetEmissiveTexture(static_cast<uint32>(value.NamedNumbers["INDEX"]));

		/* Make sure to mark the diffuse texture as an sRGB texture by default. */
		if (material.HasDiffuseTexture) result.Textures[material.DiffuseTexture].IsSRGB = true;
		result.Materials.emplace_back(material);
	}
}

void CopyTexturesToPum(const GLTFLoaderResult &input, PumIntermediate &result)
{
	GLTFImageSampler defaultSampler;

	result.Textures.reserve(input.Textures.size());
	for (const GLTFTexture &cur : input.Textures)
	{
		/* Set the image path. */
		pum_texture texture;
		texture.Identifier = input.Images[cur.Image].toUTF32();

		/* Set the sampler information, sometimes a sampler isn't set, just use a default one if this is the case. */
		const GLTFImageSampler &sampler = cur.Sampler < input.Samplers.size() ? input.Samplers[cur.Sampler] : defaultSampler;
		switch (sampler.MagFilter)
		{
		case GLTFFilter::Linear:
		case GLTFFilter::LinearMipmapNearest:
			texture.UsesLinearMagnification = true;
			break;
		case GLTFFilter::NearestMipmapLinear:
			texture.UsesLinearMipmapMode = true;
			break;
		case GLTFFilter::LinearMipmapLinear:
			texture.UsesLinearMagnification = true;
			texture.UsesLinearMipmapMode = true;
			break;
		}

		switch (sampler.MinFilter)
		{
		case GLTFFilter::Linear:
		case GLTFFilter::LinearMipmapNearest:
			texture.UsesLinaerMinification = true;
			break;
		case GLTFFilter::NearestMipmapLinear:
			texture.UsesLinearMipmapMode = true;
			break;
		case GLTFFilter::LinearMipmapLinear:
			texture.UsesLinaerMinification = true;
			texture.UsesLinearMipmapMode = true;
			break;
		}

		switch (sampler.WrapS)
		{
		case GLTFWrap::ClampToEdge:
			texture.AddressModeU = 2;
			break;
		case GLTFWrap::ClampToBorder:
			texture.AddressModeU = 3;
			break;
		case GLTFWrap::MirroredRepeat:
			texture.AddressModeU = 1;
			break;
		}

		switch (sampler.WrapT)
		{
		case GLTFWrap::ClampToEdge:
			texture.AddressModeV = 2;
			break;
		case GLTFWrap::ClampToBorder:
			texture.AddressModeV = 3;
			break;
		case GLTFWrap::MirroredRepeat:
			texture.AddressModeV = 1;
			break;
		}

		result.Textures.emplace_back(texture);
	}
}

void GltfToPum(const CLArgs & args, const GLTFLoaderResult & input, PumIntermediate & result)
{
	/* Preload all the buffer data. */
	vector<string> bufferData;
	bufferData.reserve(input.Buffers.size());
	for (const GLTFBuffer &cur : input.Buffers)
	{
		FileReader reader(cur.Uri);
		bufferData.emplace_back(reader.ReadToEnd());
	}

	CopyNodesToPum(input, result);
	CopyMeshesToPum(input, bufferData, result);
	CopyAnimationsToPum(input, bufferData, result);
	CopySkeletonToPum(input, bufferData, result);
	CopyTexturesToPum(input, result);
	CopyMaterialsToPum(input, result);
}

size_t GLTFAccessor::GetElementSize(void) const
{
	size_t componentSize;

	switch (ComponentType)
	{
	case GlTfComponentType::Int8:
	case GlTfComponentType::UInt8:
		componentSize = sizeof(int8);
		break;
	case GlTfComponentType::Int16:
	case GlTfComponentType::UInt16:
		componentSize = sizeof(int16);
		break;
	case GlTfComponentType::Int32:
	case GlTfComponentType::UInt32:
	case GlTfComponentType::Float:
		componentSize = sizeof(int32);
		break;
	case GlTfComponentType::Double:
		componentSize = sizeof(double);
		break;
	}

	switch (Type)
	{
	case GlTfType::Vector2:
		return componentSize << 1;
	case GlTfType::Vector3:
		return componentSize * 3;
	case GlTfType::Vector4:
	case GlTfType::Matrix2:
		return componentSize << 2;
	case GlTfType::Matrix3:
		return componentSize * 9;
	case GlTfType::Matrix4:
		return componentSize << 4;
	case GlTfType::Scalar:
		return componentSize;
	default:
		Log::Error("Plutonium cannot handle accessor element type!");
		return 0;
	}
}