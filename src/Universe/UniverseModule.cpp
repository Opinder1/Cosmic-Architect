#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial3D/SpatialComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"

#include "Render/RenderComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<physics3d::Components>();
		world.import<spatial3d::Components>();
		world.import<universe::Components>();

		// Initialise the spatial world of a universe
		world.observer<World, spatial3d::World>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](World& universe_world, spatial3d::World& spatial_world)
		{
			universe_world.node_entry = spatial_world.node_type.AddEntry<Node>();
		});

		world.system<const World, spatial3d::World, sim::ThreadEntityPools>(DEBUG_ONLY("UniverseLoadSpatialNode"))
			.multi_threaded()
			.term_at(2).src<sim::ThreadEntityPools>()
			.each([](flecs::entity entity, const World& universe_world, spatial3d::World& spatial_world, sim::ThreadEntityPools& entity_pools)
		{
			flecs::world stage = entity.world();

			sim::ThreadEntityPool& entity_pool = sim::GetThreadEntityPool(entity_pools, stage);

			for (spatial3d::Scale& scale : spatial_world.scales)
			{
				flecs::entity galaxy_schematic;
				
				if (!scale.load_commands.empty())
				{
					galaxy_schematic = flecs::entity(stage, entity_pool.CreateThreadEntity());

					galaxy_schematic.add<rendering::PlaceholderCube>();
				}

				for (Poly node_poly : scale.load_commands)
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
						galaxy.add<galaxy::World>();
						galaxy.set(physics3d::Position{ godot::Vector3(position_x, position_y, position_z) });
						galaxy.set(physics3d::Scale{ godot::Vector3(box_size, box_size, box_size) });
						galaxy.add<rendering::UniqueInstance>(galaxy_schematic);
						galaxy.add<rendering::Transform>();

						universe_node.entities.push_back(galaxy);
					}
				}
			}
		});

		world.system<const World, spatial3d::World, sim::ThreadEntityPools>(DEBUG_ONLY("UniverseUnloadSpatialNode"))
			.multi_threaded()
			.term_at(2).src<sim::ThreadEntityPools>()
			.each([](flecs::entity entity, const World& universe_world, spatial3d::World& spatial_world, sim::ThreadEntityPools& entity_pools)
		{
			flecs::world world = entity.world();

			sim::ThreadEntityPool& entity_pool = sim::GetThreadEntityPool(entity_pools, world);

			for (spatial3d::Scale& scale : spatial_world.scales)
			{
				for (Poly node_poly : scale.unload_commands)
				{
					spatial3d::Node& node = node_poly.GetEntry(spatial_world.node_entry);
					Node& universe_node = node_poly.GetEntry(universe_world.node_entry);

					const uint32_t entities_per_node = 4;
					const uint32_t scale_step = 1 << node.coord.scale;
					const uint32_t scale_node_step = scale_step * spatial_world.node_size;

					for (flecs::entity_t galaxy : universe_node.entities)
					{
						flecs::entity(world, galaxy).destruct();
					}
				}
			}
		});
	}
}