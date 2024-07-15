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
		ThreadEntityPool* entity_pool;

		UniverseLoadNodeCommandProcessor(flecs::world_t* world, flecs::entity_t universe_entity) :
			world(world),
			universe_entity(universe_entity),
			entity_pool(GetThreadEntityPool(world))
		{
			DEBUG_ASSERT(entity_pool != nullptr, "We could not get an entity pool for our multithread stage");
		}

		void Process(SpatialWorld3DComponent& spatial_world, UniverseScale& universe_scale, UniverseNode& universe_node)
		{
			uint32_t scale_step = 16 << universe_node.coord.scale;

			size_t add_count = 16 - universe_node.entities.size();

			flecs::entity galaxy_schematic;

			if (add_count > 0)
			{
				if (entity_pool->new_entities.size() == 0)
				{
					return;
				}

				galaxy_schematic = flecs::entity(world, entity_pool->new_entities.back());
				entity_pool->new_entities.pop_back();

				galaxy_schematic.add<RenderMesh>();
			}

			for (size_t i = 0; i < add_count; i++)
			{
				if (entity_pool->new_entities.size() == 0)
				{
					return;
				}

				flecs::entity galaxy(world, entity_pool->new_entities.back());
				entity_pool->new_entities.pop_back();

				galaxy.child_of(universe_entity);
				galaxy.add<GalaxyComponent>();

				float position_x = (float(universe_node.coord.pos.x) * scale_step) + godot::UtilityFunctions::randf_range(0, scale_step);
				float position_y = (float(universe_node.coord.pos.y) * scale_step) + godot::UtilityFunctions::randf_range(0, scale_step);
				float position_z = (float(universe_node.coord.pos.z) * scale_step) + godot::UtilityFunctions::randf_range(0, scale_step);

				galaxy.set(Position3DComponent{ godot::Vector3(position_x, position_y, position_z) });
				galaxy.add<RenderBase>(galaxy_schematic);
				galaxy.add<FlatTextureComponent>();
				galaxy.add<RenderInstance>();

				galaxy.ensure<RenderInstanceFlags>().transform = true;
				galaxy.modified<RenderInstanceFlags>();

				universe_node.entities.push_back(galaxy);
			}
		}
	};

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<PhysicsComponents>();
		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		world.singleton<UniverseComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::With);

		world.singleton<UniverseObjectComponent>()
			.add_second<UniverseComponent>(flecs::OneOf)
			.add_second<SpatialEntity3DComponent>(flecs::With);

		// Initialise the spatial world of a universe
		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.term_at(2).filter()
			.each([](const UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_ASSERT(spatial_world.max_scale == 0, "The spatial world was already initialized with a type");

			spatial_world.max_scale = 16;

			spatial_world.builder = SpatialBuilder<UniverseScale, UniverseNode>();

			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}

			spatial_world.load_command_processors.push_back(SpatialNodeCommandProcessor<UniverseLoadNodeCommandProcessor, UniverseScale, UniverseNode>());
		});

		// Uninitialize spatial world of a universe
		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.yield_existing()
			.term_at(2).filter()
			.each([](const UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}