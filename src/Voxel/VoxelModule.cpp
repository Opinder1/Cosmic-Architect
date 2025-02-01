#include "VoxelModule.h"
#include "VoxelComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

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

	void GenerateVertexesForNode(Context& context, const Node& node, godot::PackedVector3Array& array, godot::PackedColorArray& voxel_colours)
	{
		for (size_t x = 0; x < 16; x++)
		for (size_t y = 0; y < 16; y++)
		for (size_t z = 0; z < 16; z++)
		{
			Voxel voxel = node.voxels[x][y][z];
			const VoxelType& voxel_type = context.types[voxel.type];
			bool voxel_invisible = voxel_type.cache.is_invisible;

			{
				Voxel posx = node.voxels[x + 1][y][z];
				const VoxelType& posx_type = context.types[posx.type];
				bool posx_invisible = posx_type.cache.is_invisible;

				if (voxel_invisible != posx_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(0, 1, 0), godot::Vector3(0, 0, 1), voxel_invisible > posx_invisible);

					godot::Color color = voxel_invisible > posx_invisible ? voxel_type.cache.color(voxel.data) : posx_type.cache.color(posx.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}

			{
				Voxel posy = node.voxels[x][y + 1][z];
				const VoxelType& posy_type = context.types[posy.type];
				bool posy_invisible = posy_type.cache.is_invisible;

				if (voxel_invisible != posy_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 0, 1), voxel_invisible > posy_invisible);

					godot::Color color = voxel_invisible > posy_invisible ? voxel_type.cache.color(voxel.data) : posy_type.cache.color(posy.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}

			{
				Voxel posz = node.voxels[x][y][z + 1];
				const VoxelType& posz_type = context.types[posz.type];
				bool posz_invisible = posz_type.cache.is_invisible;

				if (voxel_invisible != posz_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 1, 0), voxel_invisible > posz_invisible);

					godot::Color color = voxel_invisible > posz_invisible ? voxel_type.cache.color(voxel.data) : posz_type.cache.color(posz.data);

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
			.add_second<spatial3d::WorldMarker>(flecs::With);

		// Initialise the spatial world of a universe
		world.observer<World, spatial3d::WorldMarker>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](World& voxel_world, spatial3d::WorldMarker& spatial_world)
		{
			voxel_world.node_entry = spatial_world.world.node_type.AddEntry<Node>();
			voxel_world.scale_entry = spatial_world.world.scale_type.AddEntry<Scale>();
		});
	}

	Voxel GetVoxelAtScale(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t scale)
	{
		Poly node_poly = spatial3d::GetNode(spatial_world, spatial3d::Coord(pos / 16, scale));

		if (!node_poly.IsValid())
		{
			return Voxel{};
		}

		Node& node = node_poly.GetEntry(voxel_world.node_entry);

		return node.voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelDepthFirst(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == spatial3d::k_max_world_scale)
		{
			return Voxel{};
		}

		Poly node_poly = spatial3d::GetNode(spatial_world, spatial3d::Coord(pos / 16, start_scale));

		if (!node_poly.IsValid())
		{
			return GetVoxelDepthFirst(spatial_world, voxel_world, pos, start_scale + 1);
		}

		Node& node = node_poly.GetEntry(voxel_world.node_entry);

		return node.voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelBreadthFirst(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == 0)
		{
			return Voxel{};
		}

		Poly node_poly = spatial3d::GetNode(spatial_world, spatial3d::Coord(pos / 16, start_scale));

		if (!node_poly.IsValid())
		{
			return GetVoxelDepthFirst(spatial_world, voxel_world, pos, start_scale - 1);
		}

		Node& node = node_poly.GetEntry(voxel_world.node_entry);

		return node.voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelOctreeSearch(const spatial3d::World& spatial_world, const World& voxel_world, godot::Vector3i pos, uint32_t start_scale)
	{
		godot::Vector3i node_pos = pos / 16;

		Poly node_poly = spatial3d::GetNode(spatial_world, spatial3d::Coord(node_pos, start_scale));

		if (!node_poly.IsValid())
		{
			return Voxel{};
		}

		spatial3d::Node* node = &node_poly.GetEntry(spatial_world.node_entry);

		while (1)
		{
			Poly child_node_poly = node->children[node_pos.x & 0x1][node_pos.y & 0x1][node_pos.z & 0x1];

			if (!child_node_poly.IsValid())
			{
				break;
			}

			node = &child_node_poly.GetEntry(spatial_world.node_entry);

			node_pos.x >>= 2; node_pos.y >>= 2; node_pos.z >>= 2;
		}

		Node& voxel_node = node_poly.GetEntry(voxel_world.node_entry);

		return voxel_node.voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}
}