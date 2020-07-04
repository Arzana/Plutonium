#include "Physics/Systems/PhysicalWorld2.h"
#include "Physics/Systems/MaterialDatabase.h"
#include "Physics/Systems/ConstraintSystem.h"
#include "Core/Diagnostics/Profiler.h"
#include <imgui/include/imgui.h>

#define nameof(x)			#x

Pu::PhysicalWorld2::PhysicalWorld2(void)
{
	db = new MaterialDatabase();
	sysMove = new MovementSystem();
	sysSolv = new SolverSystem(*this);
	sysCnst = new ConstraintSystem(*this);
}

Pu::PhysicalWorld2::PhysicalWorld2(PhysicalWorld2 && value)
	: db(value.db), sysMove(value.sysMove), sysCnst(value.sysCnst),
	sysSolv(value.sysSolv), searchTree(std::move(value.searchTree)),
	handleLut(std::move(value.handleLut))
{
	value.lock.lock();

	value.db = nullptr;
	value.sysMove = nullptr;
	value.sysCnst = nullptr;
	value.sysSolv = nullptr;

	value.lock.unlock();
}

Pu::PhysicalWorld2 & Pu::PhysicalWorld2::operator=(PhysicalWorld2 && other)
{
	if (this != &other)
	{
		lock.lock();
		other.lock.lock();
		Destroy();

		db = other.db;
		sysMove = other.sysMove;
		sysCnst = other.sysCnst;
		sysSolv = other.sysSolv;
		searchTree = std::move(other.searchTree);
		handleLut = std::move(other.handleLut);

		other.db = nullptr;
		other.sysMove = nullptr;
		other.sysCnst = nullptr;
		other.sysSolv = nullptr;

		other.lock.unlock();
		lock.unlock();
	}

	return *this;
}

Pu::PhysicsHandle Pu::PhysicalWorld2::AddStatic(const PhysicalObject & obj)
{
	lock.lock();
	const PhysicsHandle result = AddInternal(obj, PhysicsType::Static);
	lock.unlock();

	return result;
}

Pu::PhysicsHandle Pu::PhysicalWorld2::AddKinematic(const PhysicalObject & obj)
{
	lock.lock();
	const PhysicsHandle result = AddInternal(obj, PhysicsType::Kinematic);
	lock.unlock();

	return result;
}

Pu::PhysicsHandle Pu::PhysicalWorld2::AddMaterial(const PhysicalProperties & prop)
{
	lock.lock();
	const PhysicsHandle result = db->Add(prop);
	lock.unlock();

	return result;
}

void Pu::PhysicalWorld2::Destroy(PhysicsHandle handle)
{
	lock.lock();

#ifdef _DEBUG
	ValidateHandle(handle);
#endif

	/* Destroy the object associated with the internal handle and reset the public handle. */
	PhysicsHandle &hinternal = handleLut[physics_get_lookup_id(handle)];
	DestroyInternal(handle, hinternal);
	hinternal = PhysicsNullHandle;

	lock.unlock();
}

Pu::Matrix Pu::PhysicalWorld2::GetTransform(PhysicsHandle handle) const
{
	/* We cannot get the transform of a material... obviously. */
#ifdef _DEBUG
	ValidateHandle(handle);
	ThrowCorruptHandle(physics_get_type(handle) == PhysicsType::Material, nameof(GetTransform));
#endif

	return sysMove->GetTransform(handleLut[physics_get_lookup_id(handle)]);
}

void Pu::PhysicalWorld2::Visualize(DebugRenderer & dbgRenderer, Vector3 camPos) const
{
	if constexpr (ImGuiAvailable)
	{
		Profiler::BeginDebug();
		lock.lock();

		if (ImGui::Begin("Physical World"))
		{
			static bool showBvh = false;
			ImGui::Checkbox("Visualize BVH", &showBvh);
			if (showBvh) searchTree.Visualize(dbgRenderer);

			static bool showColliders = false;
			ImGui::Checkbox("Visualize Colliders", &showColliders);
			if (showColliders) sysCnst->Visualize(dbgRenderer, camPos);

			ImGui::Separator();
			ImGui::Text("Legend:");
			ImGui::TextColored(Color::Blue().ToVector4(), "BVH Nodes");
			ImGui::TextColored(Color::Green().ToVector4(), "Static Colliders");
			ImGui::TextColored(Color::Red().ToVector4(), "Kinematic Colliders");
			ImGui::TextColored(Color::Yellow().ToVector4(), "Cached Broadphase Colliders");

			ImGui::End();
		}

		lock.unlock();
		Profiler::End();
	}
}

void Pu::PhysicalWorld2::Update(float dt)
{
	Profiler::Begin("Physics", Color::Gray());
	lock.lock();

	const ofloat dt8 = _mm256_set1_ps(dt);

	/*
	We first check for collision events between all objects.
	This should be done early on, but it could be swapped with the next step.

	We add the constant world forces (gravity and dynamics).
	This needs to happen before we solve the collisions as they take
	the final velocity into account when applying their impulses.
	Otherwise the objects would slowely fall through floors, etc.

	Finally we solve for the collision and integrate our positions to the next timestep.
	Thus creating the new state of the world.
	*/
	sysCnst->Check();
	sysMove->ApplyGravity(dt8);
	sysMove->ApplyDrag(dt8);
	sysSolv->SolveConstriants();
	sysMove->Integrate(dt8);

	lock.unlock();
	Profiler::End();
}

void Pu::PhysicalWorld2::ThrowCorruptHandle(bool condition, const char * func)
{
	if (condition) Log::Fatal("Corrupt physics handle detected at %s::%s!", nameof(PhysicalWorld2), func);
}

void Pu::PhysicalWorld2::ValidatePhysicalObject(const PhysicalObject & obj)
{
	if (obj.Properties == PhysicsNullHandle) Log::Fatal("Physical objects must have material set!");
	if (obj.State.Mass <= 0.0f) Log::Fatal("Physical object mass must be greater than zero!");
}

Pu::PhysicsHandle Pu::PhysicalWorld2::QueryPublicHandle(PhysicsHandle handle) const
{
	for (size_t i = 0; i < handleLut.size(); i++)
	{
		if (handleLut[i] == handle) return create_physics_handle(physics_get_type(handle), i);
	}

	return PhysicsNullHandle;
}

Pu::uint16 Pu::PhysicalWorld2::QueryInternalIndex(PhysicsHandle handle) const
{
	return physics_get_lookup_id(handleLut[physics_get_lookup_id(handle)]);
}

void Pu::PhysicalWorld2::ValidateHandle(PhysicsHandle handle) const
{
	/* Check for a corrupt or internal handle. */
	if (handle == PhysicsNullHandle) Log::Fatal("Physics handle cannot be null!");
	if (handle & PhysicsHandleImplBits) Log::Fatal("Phyics handle implementation bits must be zero!");

	/* Check whether the lookup ID is plausible. */
	const uint16 i = physics_get_lookup_id(handle);
	if (i >= handleLut.size()) Log::Fatal("Unknown physics handle passed (Out of lookup table range)!");

	/* Check whether the types are equal. */
	const PhysicsType t1 = physics_get_type(handle);
	const PhysicsType t2 = physics_get_type(handleLut[i]);
	if (t1 != t2) Log::Fatal("Unknown physics handle passed (types differ)!");
}

Pu::PhysicsHandle Pu::PhysicalWorld2::AddInternal(const PhysicalObject & obj, PhysicsType type)
{
	/* Check if the user set the material. */
#ifdef _DEBUG
	ThrowCorruptHandle(obj.Properties == PhysicsNullHandle, nameof(AddInternal));
	ThrowCorruptHandle(physics_get_type(obj.Properties) != PhysicsType::Material, nameof(AddInternal));
#endif

	/* Query the material and add the object to the solver to get the index. */
	const PhysicalProperties &mat = (*db)[obj.Properties];

	size_t idx;
	if (type == PhysicsType::Static) idx = sysMove->AddItem(Matrix::CreateWorld(obj.P, obj.Theta, Vector3{ 1.0f }));
	else idx = sysMove->AddItem(obj.P, obj.V, obj.Theta, obj.Omega, obj.State.Cd, recip(obj.State.Mass));

	/* Create the public handle (to give to the user) and query the internal handle. */
	const PhysicsHandle hpublic = AllocPublicHandle(type, idx);

	/* Add the object parameters to the systems that require the handle. */
	sysSolv->AddItem(hpublic, obj.MoI, obj.State.Mass, mat.Mechanical.CoR, mat.Mechanical.CoF);
	sysCnst->AddItem(hpublic, obj.Collider.BroadPhase, obj.Collider.NarrowPhaseShape, reinterpret_cast<float*>(obj.Collider.NarrowPhaseParameters));

	return hpublic;
}

/*
The lookup table is our way of querying the actual physics objects from the lists.
The goal is to be able to resize the underlying vectors without the handle needing to change.

When an object is added we search through the lookup table for an unused spot (indicated by ~0).
If we find one we'll use that index for this object, otherwise we just emplace a new one.

When an object is removed we must go through the entire lookup table to update the indices of the objects.
*/
Pu::PhysicsHandle Pu::PhysicalWorld2::AllocPublicHandle(PhysicsType type, size_t idx)
{
	/* Check if we have ran out of address space. */
#ifdef _DEBUG
	if (idx >= maxv<uint16>()) Log::Fatal("Unable to create phyics handle (out of space)!");
#endif

	/* Search for an empty position in the lookup table. */
	for (size_t i = 0; i < handleLut.size(); i++)
	{
		if (handleLut[i] == PhysicsNullHandle)
		{
			handleLut[i] = create_physics_handle(type, idx);
			return create_physics_handle(type, i);
		}
	}

	/* No null entry was found, so just add a new one. */
	handleLut.emplace_back(create_physics_handle(type, idx));
	return create_physics_handle(type, handleLut.size() - 1);
}

void Pu::PhysicalWorld2::DestroyInternal(PhysicsHandle hpublic, PhysicsHandle hinternal)
{
	/* Destroy global parameters (order matters). */
	sysCnst->RemoveItem(hpublic);
	sysSolv->RemoveItem(hpublic);
	sysMove->RemoveItem(hinternal);

	/*
	Update all the other internal handles in the lookup table.
	The least significant bits are where the index is stored,
	so we can just branchless decrement the handle instead of creating a new one.
	The type specific bits are however done using a branch, it's just easier this way.
	*/
	const uint16 i = physics_get_lookup_id(hinternal);
	const PhysicsType t = physics_get_type(hinternal);
	for (PhysicsHandle &hcur : handleLut)
	{
		hcur -= physics_get_type(hcur) == t && physics_get_lookup_id(hcur) > i;
	}
}

void Pu::PhysicalWorld2::Destroy(void)
{
	if (sysCnst) delete sysCnst;
	if (sysSolv) delete sysSolv;
	if (sysMove) delete sysMove;
	if (db) delete db;
}