#pragma once

#include "Spatial3D/SpatialPoly.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::debugrender
{
	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	bool IsUnloadDone(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);
	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world);
	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale);
}