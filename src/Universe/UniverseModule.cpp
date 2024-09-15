#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Physics/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"

#include "Render/RenderComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	struct LoadNodeCommandProcessor
	{
		flecs::world_t* world;
		flecs::entity_t universe_entity;
		sim::ThreadEntityPool& entity_pool;

		LoadNodeCommandProcessor(flecs::world_t* world, flecs::entity_t universe_entity) :
			world(world),
			universe_entity(universe_entity),
			entity_pool(sim::GetThreadEntityPool(world))
		{

		}

		void Process(spatial::World3D& spatial_world, Scale& universe_scale, Node& universe_node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << universe_node.coord.scale;
			const uint32_t scale_node_step = scale_step * spatial_world.node_size;
			const double box_size = double(scale_step) / 2.0;

			flecs::entity galaxy_schematic(world, CreateThreadEntity(entity_pool));

			galaxy_schematic.add<rendering::PlaceholderCube>();

			for (size_t i = 0; i < entities_per_node; i++)
			{
				flecs::entity galaxy(world, CreateThreadEntity(entity_pool));

				galaxy.child_of(universe_entity);
				galaxy.add<galaxy::Galaxy>();

				float position_x = (float(universe_node.coord.pos.x) * scale_node_step); 
				float position_y = (float(universe_node.coord.pos.y) * scale_node_step);
				float position_z = (float(universe_node.coord.pos.z) * scale_node_step);

				position_x += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_y += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_z += godot::UtilityFunctions::randf_range(0, scale_node_step);

				galaxy.set(physics::Position3D{ godot::Vector3(position_x, position_y, position_z) });
				galaxy.set(physics::Scale3D{ godot::Vector3(box_size, box_size, box_size) });
				galaxy.add<rendering::UniqueInstance>(galaxy_schematic);

				universe_node.entities.push_back(galaxy);
			}
		}
	};

	struct UnloadNodeCommandProcessor
	{
		flecs::world_t* world;
		flecs::entity_t universe_entity;

		UnloadNodeCommandProcessor(flecs::world_t* world, flecs::entity_t universe_entity) :
			world(world),
			universe_entity(universe_entity)
		{}

		void Process(spatial::World3D& spatial_world, Scale& universe_scale, Node& universe_node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << universe_node.coord.scale;
			const uint32_t scale_node_step = scale_step * spatial_world.node_size;

			for (flecs::entity_t galaxy : universe_node.entities)
			{
				flecs::entity(world, galaxy).destruct();
			}
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>("UniverseModule");

		world.import<physics::Components>();
		world.import<spatial::Components>();
		world.import<universe::Components>();

		// Initialise the spatial world of a universe
		world.observer<const Universe, spatial::World3D>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](const Universe& universe, spatial::World3D& spatial_world)
		{
			DEBUG_ASSERT(!spatial_world.initialized, "The spatial world was already initialized with a type");

			spatial_world.max_scale = spatial::k_max_world_scale;

			spatial_world.node_size = 16;

			spatial_world.builder = spatial::Builder<Scale, Node>();

			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}

			spatial_world.load_command_processors.push_back(spatial::NodeCommandProcessor<LoadNodeCommandProcessor, Scale, Node>());

			spatial_world.unload_command_processors.push_back(spatial::NodeCommandProcessor<UnloadNodeCommandProcessor, Scale, Node>());

			spatial_world.initialized = true;
		});

		// Uninitialize spatial world of a universe
		world.observer<const Universe, spatial::World3D>(DEBUG_ONLY("UniverseUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.each([](const Universe& universe, spatial::World3D& spatial_world)
		{
			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}