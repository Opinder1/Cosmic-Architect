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

namespace voxel_game::spatial
{
	struct World3D;
}

namespace voxel_game::voxel
{
	struct Block;
	struct Node;

	struct Module
	{
		Module(flecs::world& world);
	};

	Block GetBlockAtScale(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t scale);

	Block GetBlockDepthFirst(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t start_scale);

	Block GetBlockBreadthFirst(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t start_scale);

	Block GetBlockOctreeSearch(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t start_scale);
}