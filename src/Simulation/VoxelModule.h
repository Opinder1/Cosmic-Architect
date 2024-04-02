#pragma once

#include <cstdint>

namespace godot
{
	struct Vector3i;
}

namespace flecs
{
	struct world;
	struct entity;
}

namespace voxel_game
{
	struct Block;
	struct VoxelWorld;

	struct VoxelModule
	{
		VoxelModule(flecs::world& world);

		static Block GetBlockAtScale(const VoxelWorld& world, godot::Vector3i pos, uint32_t scale);

		static Block GetBlockDepthFirst(const VoxelWorld& world, godot::Vector3i pos, uint32_t start_scale);

		static Block GetBlockBreadthFirst(const VoxelWorld& world, godot::Vector3i pos, uint32_t start_scale);

		static Block GetBlockOctreeSearch(const VoxelWorld& world, godot::Vector3i pos, uint32_t start_scale);
	};
}