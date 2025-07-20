#pragma once

#include "Entity/EntityPoly.h"

#include "Spatial3D/SpatialPoly.h"

namespace godot
{
	class String;
}

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::galaxy
{
	extern const entity::TypeID k_galaxy_type;

	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	bool IsUnloadDone(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);
	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world);
	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale);

	entity::Ref CreateGalaxy(Simulation& simulation, godot::Vector3 position, godot::Vector3 scale);

	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, UUID id, spatial3d::WorldPtr universe_world);
}