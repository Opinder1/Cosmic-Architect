#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyPrefabs.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Physics3D/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"

#include "Render/RenderComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	flecs::entity CreateNewUniverse(flecs::world& world, const godot::StringName& path, godot::RID scenario_id)
	{
		// Create the universe
		flecs::entity universe_entity = world.entity();

#if defined(DEBUG_ENABLED)
		universe_entity.set_name("Universe");
#endif

		universe_entity.emplace<sim::Path>(path);

		universe_entity.add<universe::World>();

		spatial3d::WorldMarker& spatial_world = universe_entity.ensure<spatial3d::WorldMarker>();
		spatial_world.world.max_scale = spatial3d::k_max_world_scale;
		spatial_world.world.node_size = 16;
		spatial_world.world.node_keepalive = 1s;

		spatial3d::InitializeWorldScales(universe_entity, spatial_world);

		if (scenario_id.is_valid())
		{
			rendering::Scenario& scenario = universe_entity.ensure<rendering::Scenario>();

			scenario.id = scenario_id;

			universe_entity.add<rendering::Transform>();
		}

		return universe_entity;
	}

	struct UniverseNodeLoader
	{
		flecs::entity entity;
		flecs::world stage;
		const World& universe_world;
		spatial3d::World& spatial_world;
		sim::ThreadEntityPool& entity_pool;

		void LoadSpatialNode(Poly node_poly)
		{
			spatial3d::Node& node = node_poly.GetEntry(spatial_world.node_entry);
			Node& universe_node = node_poly.GetEntry(universe_world.node_entry);

			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node.coord.scale;
			const double scale_node_step = scale_step * spatial_world.node_size;
			const double box_size = double(scale_step) / 2.0;

			for (size_t i = 0; i < entities_per_node; i++)
			{
				double position_x = node.coord.pos.x * scale_node_step;
				double position_y = node.coord.pos.y * scale_node_step;
				double position_z = node.coord.pos.z * scale_node_step;

				position_x += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_y += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_z += godot::UtilityFunctions::randf_range(0, scale_node_step);

				flecs::entity galaxy(stage, entity_pool.CreateThreadEntity());

				galaxy.child_of(entity);
				galaxy.is_a<galaxy::GalaxyPrefab>();
				galaxy.set(physics3d::Position{ godot::Vector3(position_x, position_y, position_z) });
				galaxy.set(physics3d::Scale{ godot::Vector3(box_size, box_size, box_size) });
				galaxy.add<rendering::Transform>();

				node.entities.push_back(galaxy);
				universe_node.galaxies.push_back(galaxy);
			}
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<physics3d::Components>();
		world.import<spatial3d::Components>();
		world.import<universe::Components>();
		world.import<galaxy::Prefabs>();

		// Initialise the spatial world of a universe
		world.observer<World, spatial3d::WorldMarker>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](World& universe_world, spatial3d::WorldMarker& spatial_world)
		{
			universe_world.node_entry = spatial_world.world.node_type.AddEntry<Node>();
			universe_world.scale_entry = spatial_world.world.scale_type.AddEntry<Scale>();
		});

		world.system<const World, spatial3d::WorldMarker, sim::ThreadEntityPools>(DEBUG_ONLY("UniverseLoadSpatialNode"))
			.multi_threaded()
			.term_at(2).src<sim::ThreadEntityPools>()
			.each([](flecs::entity entity, const World& universe_world, spatial3d::WorldMarker& spatial_world, sim::ThreadEntityPools& entity_pools)
		{
			flecs::world stage = entity.world();

			UniverseNodeLoader loader { entity, stage, universe_world, spatial_world.world, sim::GetThreadEntityPool(entity_pools, stage) };

			for (Poly scale_poly : spatial_world.world.scales)
			{
				spatial3d::Scale& scale = spatial3d::GetScale(spatial_world.world, scale_poly);

				for (Poly node_poly : scale.load_commands)
				{
					loader.LoadSpatialNode(node_poly);
				}
			}
		});

		world.system<const World, spatial3d::WorldMarker, sim::ThreadEntityPools>(DEBUG_ONLY("UniverseUnloadSpatialNode"))
			.multi_threaded()
			.term_at(2).src<sim::ThreadEntityPools>()
			.each([](flecs::entity entity, const World& universe_world, spatial3d::WorldMarker& spatial_world, sim::ThreadEntityPools& entity_pools)
		{
			flecs::world world = entity.world();

			sim::ThreadEntityPool& entity_pool = sim::GetThreadEntityPool(entity_pools, world);

			for (Poly scale_poly : spatial_world.world.scales)
			{
				spatial3d::Scale& scale = spatial3d::GetScale(spatial_world.world, scale_poly);

				for (Poly node_poly : scale.unload_commands)
				{
					spatial3d::Node& node = node_poly.GetEntry(spatial_world.world.node_entry);
					Node& universe_node = node_poly.GetEntry(universe_world.node_entry);

					const uint32_t entities_per_node = 4;
					const uint32_t scale_step = 1 << node.coord.scale;
					const uint32_t scale_node_step = scale_step * spatial_world.world.node_size;

					for (flecs::entity_t galaxy : universe_node.galaxies)
					{
						flecs::entity(world, galaxy).destruct();
					}
				}
			}
		});
	}
}