#pragma once

#include <flecs/flecs.h>

#include <cstdint>

namespace godot
{
	struct Vector3i;
}

namespace voxel_game::spatial3d
{
	struct World;
}

namespace voxel_game::voxel
{
	struct Voxel;
	struct Node;
	struct World;

	struct Module
	{
		Module(flecs::world& world);
	};

	Voxel GetVoxelAtScale(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t scale);

	Voxel GetVoxelDepthFirst(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelBreadthFirst(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t start_scale);

	Voxel GetVoxelOctreeSearch(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t start_scale);
}