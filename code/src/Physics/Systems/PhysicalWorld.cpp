#include "Physics/Systems/PhysicalWorld.h"
#include "Physics/Systems/ContactSolverSystem.h"
#include "Physics/Systems/MaterialDatabase.h"
#include "Physics/Systems/MovementSystem.h"
#include "Physics/Systems/ContactSystem.h"
#include "Core/Diagnostics/Profiler.h"

#ifdef _DEBUG
#include <imgui/include/imgui.h>
#endif

#define nameof(x)			#x

Pu::PhysicalWorld::PhysicalWorld(void)
	: Substeps(1)
{
	db = new MaterialDatabase();
	sysMove = new MovementSystem();
	sysSolv = new ContactSolverSystem(*this);
	sysCnst = new ContactSystem(*this);
}

Pu::PhysicalWorld::PhysicalWorld(PhysicalWorld && value)
	: db(value.db), sysMove(value.sysMove), sysCnst(value.sysCnst),
	sysSolv(value.sysSolv), searchTree(std::move(value.searchTree)),
	handleLut(std::move(value.handleLut)), Substeps(value.Substeps)
{
	value.lock.lock();

	value.db = nullptr;
	value.sysMove = nullptr;
	value.sysCnst = nullptr;
	value.sysSolv = nullptr;

	value.lock.unlock();
}

Pu::PhysicalWorld & Pu::PhysicalWorld::operator=(PhysicalWorld && other)
{
	if (this != &other)
	{
		lock.lock();
		other.lock.lock();
		Destroy();

		Substeps = other.Substeps;
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

Pu::PhysicsHandle Pu::PhysicalWorld::AddStatic(const PhysicalObject & obj)
{
	lock.lock();
	const PhysicsHandle result = AddInternal(obj, PhysicsType::Static);
	lock.unlock();

	return result;
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddKinematic(const PhysicalObject & obj)
{
	lock.lock();
	const PhysicsHandle result = AddInternal(obj, PhysicsType::Kinematic);
	lock.unlock();

	return result;
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddMaterial(const PhysicalProperties & prop)
{
	lock.lock();
	const PhysicsHandle result = db->Add(prop);
	lock.unlock();

	return result;
}

void Pu::PhysicalWorld::SetGravity(Vector3 g)
{
	lock.lock();
	sysMove->SetGravity(g);
	lock.unlock();
}

void Pu::PhysicalWorld::Destroy(PhysicsHandle handle)
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

Pu::Matrix Pu::PhysicalWorld::GetTransform(PhysicsHandle handle) const
{
	/* We cannot get the transform of a material... obviously. */
#ifdef _DEBUG
	ValidateHandle(handle);
	ThrowCorruptHandle(physics_get_type(handle) == PhysicsType::Material, nameof(GetTransform));
#endif

	return sysMove->GetTransform(handleLut[physics_get_lookup_id(handle)]);
}

void Pu::PhysicalWorld::Visualize(DebugRenderer & dbgRenderer, Vector3 camPos, float dt) const
{
	/*
	We only allow visualization of the physics system on debug mode.
	This is because the debugging options add a lot of overhead.
	*/
#ifdef _DEBUG
	if constexpr (ImGuiAvailable)
	{
		Profiler::BeginDebug();
		lock.lock();

		if (ImGui::Begin("Physical World"))
		{
			/* Statistics. */
			ImGui::Text("Objects:     %zu", handleLut.size());
			ImGui::Text("BVH Updates: %zu", ContactSystem::GetBVHUpdateCalls());
			ImGui::Text("Collisions:  %u/%u", ContactSystem::GetCollisionsCount(), ContactSystem::GetNarrowPhaseChecks());
			ContactSystem::ResetCounters();

			ImGui::Separator();

			/* Options. */
			static bool showBvh = false;
			ImGui::Checkbox("Visualize BVH", &showBvh);
			if (showBvh) searchTree.Visualize(dbgRenderer);

			static bool showColliders = false;
			ImGui::Checkbox("Visualize Colliders", &showColliders);
			if (showColliders) sysCnst->Visualize(dbgRenderer, camPos);

			static bool showForces = false;
			ImGui::Checkbox("Visualize Impulses", &showForces);
			if (showForces) sysMove->Visualize(dbgRenderer, dt);

			/* Legend. */
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
#else
	(void)dbgRenderer;
	(void)camPos;
	(void)dt;
#endif
}

void Pu::PhysicalWorld::Update(float dt)
{
#ifdef _DEBUG
	if (Substeps < 1) Log::Error("PhysicalWorld Substeps must be greater than zero for movement to occur!");
#endif

	if constexpr (!PhysicsProfileSystems) Profiler::Begin("Physics", Color::Gray());
	lock.lock();

	/*
	Divide the physics update into multiple substeps to get better accuracy.
	This happens on a fixed timestep to prevent sudden changes in motion.

	We first check for collision events between all objects.
	This should be done early on, but it could be swapped with the next step.

	We add the constant world forces (gravity and dynamics).
	This needs to happen before we solve the collisions as they take
	the final velocity into account when applying their impulses.
	Otherwise the objects would slowely fall through floors, etc.

	Next we check if any of our objects have come to rest.
	This is the case if the velocity is below the defined threshold.
	It's important that this threshold is smaller than the initial gravity constant,
	otherwise the initial gravity won't overcome this threshold, therefore it is set to:
	Magnitude(G) * (DeltaTime / Substeps) / 2

	Finally we solve for the collision and integrate our positions to the next timestep.
	Thus creating the new state of the world.
	*/
	const ofloat dt8 = _mm256_set1_ps(dt / Substeps);
	const ofloat threshold = _mm256_mul_ps(_mm256_mul_ps(sysMove->GetGravity().Length(), dt8), _mm256_set1_ps(0.5f));

	for (uint32 step = 0; step < Substeps; step++)
	{
		sysCnst->Check();
		sysMove->ApplyGravity(dt8);
		sysMove->ApplyDrag(dt8);
		sysSolv->SolveConstriants();
		sysMove->TrySleep(threshold);
		sysMove->Integrate(dt8);
	}

	lock.unlock();
	if constexpr (!PhysicsProfileSystems) Profiler::End();
}

void Pu::PhysicalWorld::ThrowCorruptHandle(bool condition, const char * func)
{
	if (condition) Log::Fatal("Corrupt physics handle detected at %s::%s!", nameof(PhysicalWorld2), func);
}

void Pu::PhysicalWorld::ValidatePhysicalObject(const PhysicalObject & obj)
{
	if (obj.Properties == PhysicsNullHandle) Log::Fatal("Physical objects must have material set!");
	if (obj.State.Mass <= 0.0f) Log::Fatal("Physical object mass must be greater than zero!");
}

Pu::PhysicsHandle Pu::PhysicalWorld::QueryPublicHandle(PhysicsHandle handle) const
{
	for (size_t i = 0; i < handleLut.size(); i++)
	{
		if (handleLut[i] == handle) return create_physics_handle(physics_get_type(handle), i);
	}

	return PhysicsNullHandle;
}

Pu::uint16 Pu::PhysicalWorld::QueryInternalIndex(PhysicsHandle handle) const
{
	return physics_get_lookup_id(handleLut[physics_get_lookup_id(handle)]);
}

void Pu::PhysicalWorld::ValidateHandle(PhysicsHandle handle) const
{
	/* Check for a corrupt or internal handle. */
	if (handle == PhysicsNullHandle) Log::Fatal("Physics handle cannot be null!");
	if (handle & PhysicsHandleImplBits) Log::Fatal("Physics handle implementation bits must be zero!");

	/* Check whether the lookup ID is plausible. */
	const uint16 i = physics_get_lookup_id(handle);
	if (i >= handleLut.size()) Log::Fatal("Unknown physics handle passed (Out of lookup table range)!");

	/* Check whether the types are equal. */
	const PhysicsType t1 = physics_get_type(handle);
	const PhysicsType t2 = physics_get_type(handleLut[i]);
	if (t1 != t2) Log::Fatal("Unknown physics handle passed (types differ)!");
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddInternal(const PhysicalObject & obj, PhysicsType type)
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
	else idx = sysMove->AddItem(obj.P, obj.V, obj.Theta, obj.Omega, obj.State.Cd, recip(obj.State.Mass), obj.MoI.GetInverse());

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
Pu::PhysicsHandle Pu::PhysicalWorld::AllocPublicHandle(PhysicsType type, size_t idx)
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

void Pu::PhysicalWorld::DestroyInternal(PhysicsHandle hpublic, PhysicsHandle hinternal)
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

void Pu::PhysicalWorld::Destroy(void)
{
	if (sysCnst) delete sysCnst;
	if (sysSolv) delete sysSolv;
	if (sysMove) delete sysMove;
	if (db) delete db;
}