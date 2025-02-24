#include "VoxelModule.h"
#include "VoxelComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Simulation/SimulationComponents.h"

#include <flecs/flecs.h>

#include "Util/Debug.h"

namespace voxel_game::voxel
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<spatial3d::Module>();

		world.add<CContext>();

		spatial3d::NodeType::RegisterType<Node>();
		spatial3d::ScaleType::RegisterType<Scale>();
		spatial3d::WorldType::RegisterType<World>();

		// Initialise the spatial world of a universe
		world.observer<spatial3d::CWorld>(DEBUG_ONLY("VoxelInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{
			spatial_world.types.node_type.AddType<Node>();
			spatial_world.types.scale_type.AddType<Scale>();
			spatial_world.types.world_type.AddType<World>();
		});

		world.system<spatial3d::CWorld>(DEBUG_ONLY("LoadVoxelNode"))
			.multi_threaded()
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{

		});
	}

	Voxel GetVoxelAtScale(spatial3d::Types& types, const spatial3d::World& world, godot::Vector3i pos, uint32_t scale)
	{
		const spatial3d::Node* node = spatial3d::GetNode(world, spatial3d::Coord(pos / 16, scale));

		if (node != nullptr)
		{
			return Voxel{};
		}

		return NODE_TO(node, Node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelDepthFirst(spatial3d::Types& types, const spatial3d::World& world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == spatial3d::k_max_world_scale)
		{
			return Voxel{};
		}

		const spatial3d::Node* node = spatial3d::GetNode(world, spatial3d::Coord(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetVoxelDepthFirst(types, world, pos, start_scale + 1);
		}

		return NODE_TO(node, Node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelBreadthFirst(spatial3d::Types& types, const spatial3d::World& world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == 0)
		{
			return Voxel{};
		}

		const spatial3d::Node* node = spatial3d::GetNode(world, spatial3d::Coord(pos / 16, start_scale));

		if (node == nullptr)
		{
			return GetVoxelDepthFirst(types, world, pos, start_scale - 1);
		}

		return NODE_TO(node, Node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelOctreeSearch(spatial3d::Types& types, const spatial3d::World& world, godot::Vector3i pos, uint32_t start_scale)
	{
		godot::Vector3i node_pos = pos / 16;

		const spatial3d::Node* node = spatial3d::GetNode(world, spatial3d::Coord(node_pos, start_scale));

		if (node == nullptr)
		{
			return Voxel{};
		}

		while (1)
		{
			godot::Vector3i child_pos = { node_pos.x & 0x1, node_pos.y & 0x1, node_pos.z & 0x1 };

			const spatial3d::Node* child_node = node->children[(child_pos.x * 4) + (child_pos.y * 2) + child_pos.z];

			if (child_node == nullptr)
			{
				break;
			}

			node = child_node;

			node_pos.x >>= 2; node_pos.y >>= 2; node_pos.z >>= 2;
		}

		return NODE_TO(node, Node)->voxels[pos.x % 16][pos.y % 16][pos.z % 16];
	}
}