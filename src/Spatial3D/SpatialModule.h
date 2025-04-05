#pragma once

#include "Spatial3D/SpatialPoly.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::spatial3d
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	void WorldUpdate(Simulation& simulation, WorldRef world);

	void ScaleUpdate(Simulation& simulation, ScaleRef scale);
}