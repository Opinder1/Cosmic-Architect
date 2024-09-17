#include "VoxelModule.h"
#include "VoxelComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include <flecs/flecs.h>

#include "Util/Debug.h"

namespace voxel_game::voxel
{
	const VoxelTypeCache& GetVoxelType(Context& context, uint16_t voxel_type)
	{
		auto it = context.type_cache.find(voxel_type);

		if (it == context.type_cache.end())
		{
			DEBUG_PRINT_ERROR(godot::vformat("The voxel type %d was not cached", voxel_type));
			DEBUG_CRASH();
		}

		return it->second;
	}

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

	void GenerateVertexesForNode(Context& context, const Node& node, godot::PackedVector3Array& array, godot::PackedColorArray& voxel_colours)
	{
		for (size_t x = 0; x < 16; x++)
		for (size_t y = 0; y < 16; y++)
		for (size_t z = 0; z < 16; z++)
		{
			Voxel voxel = node.voxels[x][y][z];
			const VoxelTypeCache& voxel_cache = GetVoxelType(context, voxel.type);
			bool voxel_invisible = voxel_cache.is_invisible;

			{
				Voxel posx = node.voxels[x + 1][y][z];
				const VoxelTypeCache& posx_cache = GetVoxelType(context, posx.type);
				bool posx_invisible = posx_cache.is_invisible;

				if (voxel_invisible != posx_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(0, 1, 0), godot::Vector3(0, 0, 1), voxel_invisible > posx_invisible);

					godot::Color color = voxel_invisible > posx_invisible ? voxel_cache.color(voxel.data) : posx_cache.color(posx.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}

			{
				Voxel posy = node.voxels[x][y + 1][z];
				const VoxelTypeCache& posy_cache = GetVoxelType(context, posy.type);
				bool posy_invisible = posy_cache.is_invisible;

				if (voxel_invisible != posy_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 0, 1), voxel_invisible > posy_invisible);

					godot::Color color = voxel_invisible > posy_invisible ? voxel_cache.color(voxel.data) : posy_cache.color(posy.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}

			{
				Voxel posz = node.voxels[x][y][z + 1];
				const VoxelTypeCache& posz_cache = GetVoxelType(context, posz.type);
				bool posz_invisible = posz_cache.is_invisible;

				if (voxel_invisible != posz_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 1, 0), voxel_invisible > posz_invisible);

					godot::Color color = voxel_invisible > posz_invisible ? voxel_cache.color(voxel.data) : posz_cache.color(posz.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}
		}
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<spatial3d::Module>();

		world.add<Context>();

		world.singleton<World>()
			.add_second<spatial3d::World>(flecs::With);
	}

	Voxel GetVoxelAtScale(const spatial3d::World& spatial_world, godot::Vector3i pos, uint32_t scale)
	{
		spatial3d::Node* node = spatial3d::GetNode(spatial_world, spatial3d::Coord(pos / 16, scale));

		if (node == nullptr)
		{
			return Voxel{};
		}

		return static_cast<Node*>(node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelDepthFirst(const spatial3d::World& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == spatial3d::k_max_world_scale)
		{
			return Voxel{};
		}

		spatial3d::Node* node = spatial3d::GetNode(spatial_world, spatial3d::Coord(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetVoxelDepthFirst(spatial_world, pos, start_scale + 1);
		}

		return static_cast<Node*>(node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelBreadthFirst(const spatial3d::World& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == 0)
		{
			return Voxel{};
		}

		spatial3d::Node* node = spatial3d::GetNode(spatial_world, spatial3d::Coord(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetVoxelDepthFirst(spatial_world, pos, start_scale - 1);
		}

		return static_cast<Node*>(node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelOctreeSearch(const spatial3d::World& spatial_world, godot::Vector3i pos, uint32_t start_scale)
	{
		godot::Vector3i node_pos = pos / 16;

		spatial3d::Node* node = spatial3d::GetNode(spatial_world, spatial3d::Coord(node_pos, start_scale));

		if (node == nullptr)
		{
			return Voxel{};
		}

		while (1)
		{
			spatial3d::Node* child_node = node->children[node_pos.x & 0x1][node_pos.y & 0x1][node_pos.z & 0x1];

			if (child_node == nullptr)
			{
				break;
			}

			node = child_node;

			node_pos.x >>= 2; node_pos.y >>= 2; node_pos.z >>= 2;
		}

		return static_cast<Node*>(node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}
}