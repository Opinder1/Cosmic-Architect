#pragma once

#include <flecs/flecs.h>

#include <cstdint>

namespace godot
{
	struct Vector3i;
}

namespace voxel_game::spatial3d
{
	struct ConstWorldRef;
}

namespace voxel_game::voxel
{
	struct Voxel;

	struct Module
	{
		Module(flecs::world& world);
	};

	Voxel GetVoxelAtScale(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t scale);

	Voxel GetVoxelDepthFirst(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelBreadthFirst(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelOctreeSearch(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t start_scale);
}