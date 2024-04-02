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

	struct VoxelModule
	{
		VoxelModule(flecs::world& world);

		static Block GetBlockAtScale(flecs::entity world, godot::Vector3i pos, uint32_t scale);

		static Block GetBlockDepthFirst(flecs::entity world, godot::Vector3i pos, uint32_t start_scale);

		static Block GetBlockBreadthFirst(flecs::entity world, godot::Vector3i pos, uint32_t start_scale);

		static Block GetBlockOctreeSearch(flecs::entity world, godot::Vector3i pos, uint32_t start_scale);
	};
}