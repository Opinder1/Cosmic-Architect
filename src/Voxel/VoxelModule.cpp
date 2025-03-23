#include "VoxelModule.h"
#include "VoxelComponents.h"
#include "VoxelWorld.h"

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

		world.system<spatial3d::CWorld>(DEBUG_ONLY("LoadVoxelNode"))
			.multi_threaded()
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{

		});
	}

	Voxel GetVoxelAtScale(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t scale)
	{
		spatial3d::ConstNodeRef node = spatial3d::GetNode(world, pos / 16, scale);

		if (node)
		{
			return Voxel{};
		}

		return (node->*&Node::voxels)[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelDepthFirst(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == spatial3d::k_max_world_scale)
		{
			return Voxel{};
		}

		spatial3d::ConstNodeRef node = spatial3d::GetNode(world, pos / 16, start_scale);

		if (node)
		{
			return GetVoxelDepthFirst(world, pos, start_scale + 1);
		}

		return (node->*&Node::voxels)[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelBreadthFirst(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t start_scale)
	{
		if (start_scale == 0)
		{
			return Voxel{};
		}

		spatial3d::ConstNodeRef node = spatial3d::GetNode(world, pos / 16, start_scale);

		if (node)
		{
			return GetVoxelDepthFirst(world, pos, start_scale - 1);
		}

		return (node->*&Node::voxels)[pos.x % 16][pos.y % 16][pos.z % 16];
	}

	Voxel GetVoxelOctreeSearch(spatial3d::ConstWorldRef world, godot::Vector3i pos, uint32_t start_scale)
	{
		godot::Vector3i node_pos = pos / 16;

		spatial3d::ConstNodeRef node = spatial3d::GetNode(world, node_pos, start_scale);

		if (node)
		{
			return Voxel{};
		}

		while (1)
		{
			godot::Vector3i child_pos = { node_pos.x & 0x1, node_pos.y & 0x1, node_pos.z & 0x1 };

			spatial3d::ConstNodeRef child_node = (node->*&spatial3d::Node::children)[(child_pos.x * 4) + (child_pos.y * 2) + child_pos.z];

			if (child_node)
			{
				break;
			}

			node = child_node;

			node_pos.x >>= 2; node_pos.y >>= 2; node_pos.z >>= 2;
		}

		return (node->*&Node::voxels)[pos.x % 16][pos.y % 16][pos.z % 16];
	}
}