#pragma once

#include "Spatial3D/SpatialPoly.h"

#include <cstdint>

namespace godot
{
	struct Vector3i;
}

namespace voxel_game::universe
{
	struct Simulation;
}

namespace voxel_game::voxel
{
	struct Voxel;

	void Initialize(universe::Simulation& simulation);

	void Uninitialize(universe::Simulation& simulation);

	void Update(universe::Simulation& simulation);

	void WorkerUpdate(universe::Simulation& simulation, size_t index);

	Voxel GetVoxelAtScale(spatial3d::WorldRef world, godot::Vector3i pos, uint32_t scale);

	Voxel GetVoxelDepthFirst(spatial3d::WorldRef world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelBreadthFirst(spatial3d::WorldRef world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelOctreeSearch(spatial3d::WorldRef world, godot::Vector3i pos, uint32_t start_scale);
}