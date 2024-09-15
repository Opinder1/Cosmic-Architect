#include "VoxelModule.h"
#include "VoxelComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialModule.h"

#include <flecs/flecs.h>

#include "Util/Debug.h"

namespace voxel_game::voxel
{
	void AddSquare(godot::PackedVector3Array& array, godot::Vector3 origin, godot::Vector3 right, godot::Vector3 up, bool reverse)
	{
		godot::Vector3 bottom_left = origin;
		godot::Vector3 bottom_right = origin + right;
		godot::Vector3 top_left = origin + up;
		godot::Vector3 top_right = origin + up + right;

		if (reverse)
		{
			array.push_back(bottom_left);
			array.push_back(bottom_right);
			array.push_back(top_right);

			array.push_back(top_right);
			array.push_back(top_left);
			array.push_back(bottom_left);
		}
		else
		{
			array.push_back(bottom_left);
			array.push_back(top_left);
			array.push_back(top_right);

			array.push_back(top_right);
			array.push_back(bottom_right);
			array.push_back(bottom_left);
		}
	}

	void GenerateVertexesForNode(const Node& node, godot::PackedVector3Array& array, std::vector<Block>& block_ids)
	{
		for (size_t x = 0; x < 16; x++)
		for (size_t y = 0; y < 16; y++)
		for (size_t z = 0; z < 16; z++)
		{
			Block block = node.blocks[x][y][z];
			bool block_air = block.type == 0;

			Block posx = node.blocks[x + 1][y][z];
			bool posx_air = posx.type == 0;

			if (block_air != posx_air)
			{
				AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(0, 1, 0), godot::Vector3(0, 0, 1), block_air > posx_air);
				block_ids.insert(block_ids.end(), 6, block);
			}

			Block posy = node.blocks[x][y + 1][z];
			bool posy_air = posy.type == 0;

			if (block_air != posy_air)
			{
				AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 0, 1), block_air > posy_air);
				block_ids.insert(block_ids.end(), 6, block);
			}

			Block posz = node.blocks[x][y][z + 1];
			bool posz_air = posz.type == 0;

			if (block_air != posz_air)
			{
				AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 1, 0), block_air > posz_air);
				block_ids.insert(block_ids.end(), 6, block);
			}
		}
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<spatial::Module>();

		world.singleton<World>()
			.add_second<spatial::World3D>(flecs::With);
	}

	Block GetBlockAtScale(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t scale)
	{
		spatial::Node3D* node = spatial::GetNode(spatial_world, spatial::Coord3D(pos / 16, scale));

		if (node == nullptr)
		{
			return Block{};
		}

		return static_cast<Node*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Block GetBlockDepthFirst(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == spatial::k_max_world_scale)
		{
			return Block{};
		}

		spatial::Node3D* node = spatial::GetNode(spatial_world, spatial::Coord3D(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetBlockDepthFirst(spatial_world, pos, start_scale + 1);
		}

		return static_cast<Node*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Block GetBlockBreadthFirst(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == 0)
		{
			return Block{};
		}

		spatial::Node3D* node = spatial::GetNode(spatial_world, spatial::Coord3D(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetBlockDepthFirst(spatial_world, pos, start_scale - 1);
		}

		return static_cast<Node*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Block GetBlockOctreeSearch(const spatial::World3D& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		godot::Vector3i node_pos = pos / 16;

		spatial::Node3D* node = spatial::GetNode(spatial_world, spatial::Coord3D(node_pos, start_scale));

		if (node == nullptr)
		{
			return Block{};
		}

		while (1)
		{
			spatial::Node3D* child_node = node->children[node_pos.x & 0x1][node_pos.y & 0x1][node_pos.z & 0x1];

			if (child_node == nullptr)
			{
				break;
			}

			node = child_node;

			node_pos.x >>= 2; node_pos.y >>= 2; node_pos.z >>= 2;
		}

		return static_cast<Node*>(node)->blocks[pos.x % 16][pos.y % 16][pos.z % 16];
	}
}