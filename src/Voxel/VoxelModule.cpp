#include "VoxelModule.h"
#include "VoxelComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Simulation/SimulationComponents.h"

#include <flecs/flecs.h>

#include "Util/Debug.h"

namespace voxel_game::voxel
{
	// Spawns a bunch of flat squares around the camera on the xz plane
	struct VoxelNodeLoader
	{
		flecs::entity entity;
		flecs::world stage;
		const World& voxel_world;
		spatial3d::World& spatial_world;
		sim::ThreadEntityPool& entity_pool;

		void LoadNode(Poly node_poly)
		{
			spatial3d::Node& node = node_poly.GetEntry(spatial_world.node_entry);
			Node& voxel_node = node_poly.GetEntry(voxel_world.node_entry);


		}
	};

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

		world.system<const World, spatial3d::WorldMarker, sim::ThreadEntityPools>(DEBUG_ONLY("LoadVoxelNode"))
			.multi_threaded()
			.term_at(2).src<sim::ThreadEntityPools>()
			.each([](flecs::entity entity, const World& voxel_world, spatial3d::WorldMarker& spatial_world, sim::ThreadEntityPools& entity_pools)
		{
			flecs::world stage = entity.world();

			VoxelNodeLoader loader{ entity, stage, voxel_world, spatial_world.world, sim::GetThreadEntityPool(entity_pools, stage) };

			for (Poly scale_poly : spatial_world.world.scales)
			{
				spatial3d::Scale& scale = spatial3d::GetScale(spatial_world.world, scale_poly);

				for (Poly node_poly : scale.load_commands)
				{
					loader.LoadNode(node_poly);
				}
			}
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
			godot::Vector3i child_pos = { node_pos.x & 0x1, node_pos.y & 0x1, node_pos.z & 0x1 };

			Poly child_node_poly = node->children[(child_pos.x * 4) + (child_pos.y * 2) + child_pos.z];

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