#pragma once

#include "Spatial3D/SpatialPoly.h"

#include <cstdint>

namespace godot
{
	struct Vector3i;
}

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::voxel
{
	struct Voxel;

	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	Voxel GetVoxelAtScale(spatial3d::WorldPtr world, godot::Vector3i pos, uint32_t scale);

	Voxel GetVoxelDepthFirst(spatial3d::WorldPtr world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelBreadthFirst(spatial3d::WorldPtr world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelOctreeSearch(spatial3d::WorldPtr world, godot::Vector3i pos, uint32_t start_scale);
}