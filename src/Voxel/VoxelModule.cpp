#include "VoxelModule.h"
#include "VoxelComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialModule.h"

#include <flecs/flecs.h>

#include "Util/Debug.h"

namespace voxel_game
{
	VoxelModule::VoxelModule(flecs::world& world)
	{
		world.module<VoxelModule>();

		world.import<VoxelComponents>();
		world.import<SpatialModule>();
	}

	Block VoxelModule::GetBlockAtScale(const SpatialWorld3DComponent& spatial_world, godot::Vector3i pos, uint32_t scale)
	{
		SpatialNode3D* node = SpatialModule::GetNode(spatial_world, SpatialCoord3D(pos / 16, scale));

		if (node == nullptr)
		{
			return Block{};
		}

		return static_cast<VoxelNode*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Block VoxelModule::GetBlockDepthFirst(const SpatialWorld3DComponent& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == k_max_world_scale)
		{
			return Block{};
		}

		SpatialNode3D* node = SpatialModule::GetNode(spatial_world, SpatialCoord3D(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetBlockDepthFirst(spatial_world, pos, start_scale + 1);
		}

		return static_cast<VoxelNode*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Block VoxelModule::GetBlockBreadthFirst(const SpatialWorld3DComponent& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == 0)
		{
			return Block{};
		}

		SpatialNode3D* node = SpatialModule::GetNode(spatial_world, SpatialCoord3D(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetBlockDepthFirst(spatial_world, pos, start_scale - 1);
		}

		return static_cast<VoxelNode*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Block VoxelModule::GetBlockOctreeSearch(const SpatialWorld3DComponent& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		godot::Vector3i node_pos = pos / 16;

		SpatialNode3D* node = SpatialModule::GetNode(spatial_world, SpatialCoord3D(node_pos, start_scale));

		if (node == nullptr)
		{
			return Block{};
		}

		while (1)
		{
			SpatialNode3D* child_node = node->children[node_pos.x & 0x1][node_pos.y & 0x1][node_pos.z & 0x1];

			if (child_node == nullptr)
			{
				break;
			}

			node = child_node;

			node_pos.x >>= 2; node_pos.y >>= 2; node_pos.z >>= 2;
		}

		return static_cast<VoxelNode*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}
}