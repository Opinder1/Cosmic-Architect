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

namespace voxel_game
{
	struct UniverseLoadNodeCommandProcessor
	{
		flecs::world_t* world;
		flecs::entity_t universe_entity;
		ThreadEntityPool& entity_pool;

		UniverseLoadNodeCommandProcessor(flecs::world_t* world, flecs::entity_t universe_entity) :
			world(world),
			universe_entity(universe_entity),
			entity_pool(GetThreadEntityPool(world))
		{

		}

		void Process(SpatialWorld3DComponent& spatial_world, UniverseScale& universe_scale, UniverseNode& universe_node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << universe_node.coord.scale;
			const uint32_t scale_node_step = scale_step * spatial_world.node_size;
			const double box_size = double(scale_step) / 2.0;

			flecs::entity galaxy_schematic(world, CreateThreadEntity(entity_pool));

			galaxy_schematic.add<RenderMesh>();

			for (size_t i = 0; i < entities_per_node; i++)
			{
				flecs::entity galaxy(world, CreateThreadEntity(entity_pool));

				galaxy.child_of(universe_entity);
				galaxy.add<GalaxyComponent>();

				float position_x = (float(universe_node.coord.pos.x) * scale_node_step); 
				float position_y = (float(universe_node.coord.pos.y) * scale_node_step);
				float position_z = (float(universe_node.coord.pos.z) * scale_node_step);

				position_x += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_y += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_z += godot::UtilityFunctions::randf_range(0, scale_node_step);

				galaxy.set(Position3DComponent{ godot::Vector3(position_x, position_y, position_z) });
				galaxy.set(Scale3DComponent{ godot::Vector3(box_size, box_size, box_size) });
				galaxy.add<UniqueRenderInstance>(galaxy_schematic);
				galaxy.add<RenderTreeNode>();

				universe_node.entities.push_back(galaxy);
			}
		}
	};

	struct UniverseUnloadNodeCommandProcessor
	{
		flecs::world_t* world;
		flecs::entity_t universe_entity;

		UniverseUnloadNodeCommandProcessor(flecs::world_t* world, flecs::entity_t universe_entity) :
			world(world),
			universe_entity(universe_entity)
		{}

		void Process(SpatialWorld3DComponent& spatial_world, UniverseScale& universe_scale, UniverseNode& universe_node)
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

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<PhysicsComponents>();
		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		// Initialise the spatial world of a universe
		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](const UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_ASSERT(!spatial_world.initialized, "The spatial world was already initialized with a type");

			spatial_world.max_scale = k_max_world_scale;

			spatial_world.node_size = 16;

			spatial_world.builder = SpatialBuilder<UniverseScale, UniverseNode>();

			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}

			spatial_world.load_command_processors.push_back(SpatialNodeCommandProcessor<UniverseLoadNodeCommandProcessor, UniverseScale, UniverseNode>());

			spatial_world.unload_command_processors.push_back(SpatialNodeCommandProcessor<UniverseUnloadNodeCommandProcessor, UniverseScale, UniverseNode>());

			spatial_world.initialized = true;
		});

		// Uninitialize spatial world of a universe
		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.each([](const UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}