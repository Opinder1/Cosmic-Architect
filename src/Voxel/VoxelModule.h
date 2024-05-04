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
	struct SpatialWorld3DComponent;

	struct VoxelModule
	{
		VoxelModule(flecs::world& world);

		static Block GetBlockAtScale(const SpatialWorld3DComponent& world, godot::Vector3i pos, uint32_t scale);

		static Block GetBlockDepthFirst(const SpatialWorld3DComponent& world, godot::Vector3i pos, uint32_t start_scale);

		static Block GetBlockBreadthFirst(const SpatialWorld3DComponent& world, godot::Vector3i pos, uint32_t start_scale);

		static Block GetBlockOctreeSearch(const SpatialWorld3DComponent& world, godot::Vector3i pos, uint32_t start_scale);
	};
}