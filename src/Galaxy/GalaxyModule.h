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
	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	bool IsUnloadDone(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);
	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world);
	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale);

	entity::Ref CreateGalaxy(Simulation& simulation, spatial3d::NodePtr node, godot::Vector3 position, godot::Vector3 scale, spatial3d::WorldPtr universe_world);

	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, UUID id, spatial3d::WorldPtr universe_world);
}