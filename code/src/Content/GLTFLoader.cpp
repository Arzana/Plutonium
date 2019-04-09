#include "Content/GLTFLoader.h"
#include "Streams/FileReader.h"
#include "Streams/BinaryReader.h"
#include <nlohmann/fifo_map.hpp>
#include <nlohmann/json.hpp>
#include "Config.h"

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

namespace Pu
{
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
	void HandleJsonDefaultScene(const json &scene, GLTFFile &file)
	{
		/* Scene is a single index value to a scene so simply parse that. */
		if (scene.is_number_unsigned()) file.DefaultScene = scene;
		else LogCorruptJsonHeader("default scene cannot be interpreted as an index");
	}

	/* Parses all scenes. */
	void HandleJsonScenes(const json &scenes, GLTFFile &file)
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
	void HandleJsonNodes(const json &nodes, GLTFFile &file)
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

	void HandleJsonMeshes(const json &meshes, GLTFFile &file)
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

	void HandleJsonAnimations(const json &animations, GLTFFile &file)
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

	void HandleJsonSkins(const json &skins, GLTFFile &file)
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

	void HandleJsonAccessors(const json &accessors, GLTFFile &file)
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
						if (type == "VEC2") accessor.FieldType.ContainerType = SizeType::Vector2;
						else if (type == "VEC3") accessor.FieldType.ContainerType = SizeType::Vector3;
						else if (type == "VEC4") accessor.FieldType.ContainerType = SizeType::Vector4;
						else if (type == "MAT2") accessor.FieldType.ContainerType = SizeType::Matrix2;
						else if (type == "MAT3") accessor.FieldType.ContainerType = SizeType::Matrix3;
						else if (type == "MAT4") accessor.FieldType.ContainerType = SizeType::Matrix4;
					}
					else LogCorruptJsonHeader("accessor type isn't a string");
				}
				else if (upperKey == "COMPONENTTYPE")
				{
					if (val.is_number_unsigned()) accessor.FieldType.ComponentType = static_cast<ComponentType>(val);
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

	void HandleJsonMaterials(const json &materials, GLTFFile &file)
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

	void HandleJsonTextures(const json &textures, GLTFFile &file)
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

	void HandleJsonImages(const json &images, GLTFFile &file)
	{
		CHECK_IF_ARRAY(images);

		/* Loop through all images in the array. */
		file.Images.reserve(images.size());
		for (const json &cur : images)
		{
			/* Images must be objects. */
			GLTFImage image;
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
				if (string(key).toUpper() == "URI")
				{
					if (val.is_string()) image.Uri = string(val).toWide();
					else LogCorruptJsonHeader("Image URI isn't a valid path");
				}
				else Log::Warning("GLTF loader doesn't currently handle embedded images!");
			}

			file.Images.emplace_back(image);
		}
	}

	void HandleJsonImageSamplers(const json &samplers, GLTFFile &file)
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

	void HandleJsonBufferViews(const json &bufferViews, GLTFFile &file)
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

	void HandleJsonBuffers(const json &buffers, GLTFFile &file)
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

	void LoadJsonGLTF(const string &source, GLTFFile &file)
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
}

void Pu::_CrtLoadGLTF(const wstring & path, GLTFFile & file)
{
	/* Make sure the file is valid. */
	const wstring ext = path.fileExtension().toUpper();
	const wstring dir = path.fileDirectory();

	if (ext == L"GLTF")
	{
		/* Only load the header of the GLTF from Json format. */
		FileReader reader(path);
		LoadJsonGLTF(reader.ReadToEnd(), file);
	}
	else if (ext == L"GLB")
	{
		/* Should implement this for faster loading at some point. */
		Log::Error("Unable to load binary GLTF at this time!");
	}
	else Log::Error("Attempting to load .%ls file '%ls' as GLTF file!", ext.c_str(), path.fileNameWithoutExtension().c_str());

	/* Add the file's directory to all buffers and images. */
	for (GLTFBuffer &cur : file.Buffers) cur.Uri = dir + cur.Uri;
	for (GLTFImage &cur : file.Images) cur.Uri = dir + cur.Uri;
}