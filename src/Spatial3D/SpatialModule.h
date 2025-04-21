#pragma once

#include "Spatial3D/SpatialPoly.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::spatial3d
{
	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);
	void WorldUpdate(Simulation& simulation, WorldPtr world);
	void ScaleUpdate(Simulation& simulation, ScalePtr scale);
}