/*
Defines a UTF-32 string.
Strings should not be used often thusly saving them in UTF-8 will save almost no bytes
whilst bringing a big parsing cost to the loader.
*/
struct STR
{
	unsigned int Length;
	int Characters[Length];
};

struct VEC3
{
	float X;
	float Y;
	float Z;
};

struct QUAT
{
	float I;
	float J;
	float K;
	float R;
};

struct MAT4
{
	float f[16];
};

struct BOX
{
	VEC3 LowerBound;
	VEC3 UpperBound;
};

struct Color
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

/*
A node is a point in the model file that can be represented by a transform in the local space.
The only thing it needs are the child count and children (if the child count > 0).
Al other things are only present if their flag is set.
The flags currently contain the following values:
	- 0x01: whether a mesh index is present.
	- 0x02: whether a skin index is present.
	- 0x04: whether a translation is present.
	- 0x08: whether a rotation is present.
	- 0x10: whether a scale is present.

	 0 1 2 3 4 5 6 7
	+-+-+-+-+-+-+-+-+
	|M|S|T|R|S| Free|
	+-+-+-+-+-+-+-+-+
*/
struct NODE
{
	unsigned int ChildCount;
	unsigned int Children[ChildCount];	// Indices to child nodes.
	unsigned char Flags;

	unsigned int Mesh;					// Index to the mesh.
	unsigned int Skin;					// Index to the animation skin

	VEC3 Translation;
	QUAT Rotation;
	VEC3 Scale;
};

/*
A mesh contains the static geometry data for a model positions are required but al other data is optional.
The flags currently contain the following values:
	- 0x01: whether a material index is present.
	- 0x02: whether the mesh contains normals.
	- 0x04: whether the mesh contains tangents.
	- 0x08: whether the mesh contains texture coordinates.
	- 0x10: whether the mesh contains vertex colors.
	- 0x20 & 0x40: whether the mesh contains weighted joints:
		0x0: no
		0x1: uint8 joint indices, float weights
		0x2: uint16 joint indices, float weights
	- 0x80 & 0x100: The index mode: 
		0x0: none
		0x1: uint16
		0x2: uint32
	- 0x200 & 0x400 & 0x800: The render topology:
		0x0: points
		0x1: lines
		0x2: line strip
		0x3: triangles
		0x4: traingle strip
		0x5: traingle fan

		 0               1              
		 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|M|N|T|T|V|J W|I M| TOP | Free  |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct MESH
{
	STR Identifier;
	unsigned short int Flags;
	BOX Bounds;

	unsigned long long VertexViewStart;
	unsigned long long VertexViewSize;
	unsigned int Material;				// Optional
	unsigned long long IndexViewStart;	// Optional
	unsigned long long IndexViewSize;	// Optional
};

/* Defines a single animation frame for a specific node. */
struct FRAME
{
	float Time;							// In seconds.
	VEC3 Translation;
	QUAT Rotation;
	VEC3 Scale;
	BOX Bounds;							// Precalculated to increase performance.
};

/* 
Defines the part of an animation relative to a specific node.
Most animations will have a single sequence.
*/
struct SEQ
{
	unsigned int Node;
	unsigned int FrameCount;
	FRAME Frames[FrameCount];
};

/*
Defines a named animation that can either be skeletal or morph.
An animation must either have sequences set or mesh frames set.
Sequences defined the frames per node for a premade mesh.
Mesh frames defined a completely new mesh to sample from instead of the default mesh.
The duration (for skeletal) can be calculated by querying the highest timestamp in the frames.
Current flags:
	- 0x01 & 0x2 & 0x4: Interpolation mode
		0x0: None
		0x1: Linear
		0x2: Cubic		Args: Tan1, Tan2
		0x3: Spring		Args: Stiffness, Damping
	- 0x08: looping
	- 0x10: reverse
	- 0x20: animation type:
		0x0: skeletal	Seqences will be defined
		0x1: morph		Frames and Duraction will be defined
	- 0x40: should bake

	 0 1 2 3 4 5 6 7
	+-+-+-+-+-+-+-+-+
	|I P M|L|R|T|B| |
	+-+-+-+-+-+-+-+-+
*/
struct ANIM
{
	STR Identifier;
	unsigned char Flags;
	float Arg1;
	float Arg2;

	unsigned int Count;
	SEQ Sequences[Count];				// Optional
	unsigned int Frames[Count];			// Optional
	float Duration;						// Optional
};

/* A joint defined a single point in a skeleton with its associated node and inverse bind matrix. */
struct JOINT
{
	unsigned int Node;
	MAT4 IBind;
};

/*
A skeleton defined a set of joints matrices that are bound to nodes.
*/
struct SKLT
{
	STR Identifier;
	unsigned int Root;
	unsigned int JointCount;
	JOINT Joints[JointCount];
};

/*
Defines a PBR specular glossiness material.
Currently flags contains the following values:
	- 0x01: double sided
	- 0x02 & 0x04: alpha mode:
		0x0: no blending
		0x1: fully opaque if < threshold; discard if > threshold
		0x2: Porter & Duff blending
	- 0x08: diffuse texture present
	- 0x10: speculargloss texture present
	- 0x20: normal texture present.
	- 0x40: occlusion texture present.
	- 0x80: emissive texture present.

	 0 1 2 3 4 5 6 7
	+-+-+-+-+-+-+-+-+
	|D|A M|D|S|N|O|E|
	+-+-+-+-+-+-+-+-+
*/
struct MTLS
{
	STR Identifier;
	unsigned char Flags;

	CLR DiffuseFactor;
	CLR SpecularFactor;
	CLR EmissiveFactor;
	float Glossiness;
	float EmissiveIntensity;			// Used as a scalar for emissive color, to get it up to HDR color space.

	float AlphaThreshold;				// Optional.
	unsigned int DiffuseTexture;		// Optional.
	unsigned int SpecGlossTexture;		// Optional.
	unsigned int NormalTexture;			// Optional.
	unsigned int OcclusionTexture;		// Optional.
	unsigned int EmissiveTexture;		// Optional.
};

/*
A texture defined an additional image source to load and the flags defined how the image should be sampled.
Current flags include:
	- 0x01: magnification filter (0 for near, 1 for linear)
	- 0x02: minification filter (0 for near, 1 for linear)
	- 0x04: mipmap mode (0 for near, 1 for linear)
	- 0x08 & 0x10: address mode U:
		0x0: repeat
		0x1: mirrored repeat
		0x2: clamp to edge
		0x3: clamp to border
	- 0x20 & 0x40: address mode V:
		0x0: repeat
		0x1: mirrored repeat
		0x2: clamp to edge
		0x3: clamp to border
*/
struct TEXT
{
	STR ImagePath;
	unsigned char Flags;
};

/*
A Plutonium Model (.pum) file is always saved using Little Endian.
*/
struct PUM
{
	char MagicNumber[4];				// PUM0
	unsigned int Version;				// Packed as Vulkan version numbers
	STR Identifier;						// Used as a display string instead of the file name.

	struct Amounts						
	{
		unsigned int NodeCount;
		unsigned int MeshCount;
		unsigned int AnimationCount;
		unsigned int SkeletonCount;
		unsigned int MaterialCount;
		unsigned int TextureCount;
	};

	struct Offsets						// These are used to easily skip items if they are not needed they are only added to the file if the associated count > 0.
	{
		unsigned long long NodeOffset;
		unsigned long long MeshOffset;
		unsigned long long AnimationOffset;
		unsigned long long SkeletonOffset;
		unsigned long long MaterialOffset;
		unsigned long long TextureOffset;
		unsigned long long BufferOffset;
		unsigned long long BufferSize;
	};

	NODE Nodes[NodeCount];
	MESH Geometry[MeshCount];
	ANIM Animations[AnimationCount];
	SKLT Skeletons[SkinCount];
	MTLS Materials[MaterialCount];
	TEXT Textures[TextureCount];
	unsigned char Data[BufferSize];		// Contains the vertex and index data in a format that's easy to stage to the GPU.
};