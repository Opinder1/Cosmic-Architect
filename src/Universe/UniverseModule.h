#pragma once

#include "Entity/EntityPoly.h"

#include "Spatial3D/SpatialPoly.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::universe
{
	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	bool IsUnloadDone(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);
	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world);
	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale);

	entity::Ref GetUniverse(Simulation& simulation);
	void LoadUniverse(Simulation& simulation, const godot::String& path);
}