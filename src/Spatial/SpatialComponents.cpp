#include "SpatialComponents.h"

#include "Physics/PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// Add a cached query for all of a spatial worlds child nodes for fast access
	void WorldAddChildQuery(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		spatial_world.commands_query = entity.world().query_builder<SpatialCommands3DComponent>()
			.term(flecs::ChildOf, entity)
			.build();
	}

	// Clean up the commands query when destroying a world
	void WorldRemoveChildQuery(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		spatial_world.commands_query.destruct();
	}

	SpatialComponents::SpatialComponents(flecs::world& world)
	{
		world.module<SpatialComponents>();

		world.import<PhysicsComponents>();

		// Components
		world.component<SpatialEntity3DComponent>();
		world.component<SpatialScale3DComponent>();
		world.component<SpatialWorld3DComponent>();
		world.component<SpatialCommands3DComponent>();
		world.component<SpatialScaleThread3DComponent>();
		world.component<SpatialRegionThread3DComponent>();
		world.component<SpatialNodeThread3DComponent>();
		world.component<SpatialLoader3DComponent>();

		// Relationships
		world.component<SpatialEntity3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialScale3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialScaleThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf).add_second<SpatialCommands3DComponent>(flecs::With);
		world.component<SpatialRegionThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf).add_second<SpatialCommands3DComponent>(flecs::With);
		world.component<SpatialNodeThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf).add_second<SpatialCommands3DComponent>(flecs::With);
		world.component<SpatialLoader3DComponent>().add_second<SpatialCommands3DComponent>(flecs::With).add_second<Position3DComponent>(flecs::With).add_second<SpatialScale3DComponent>(flecs::With);

		// Phases
		world.entity<WorldLoaderProgressPhase>().add(flecs::Phase).depends_on(flecs::OnUpdate);
		world.entity<WorldNodeProgressPhase>().add(flecs::Phase).depends_on<WorldLoaderProgressPhase>();
		world.entity<WorldRegionProgressPhase>().add(flecs::Phase).depends_on<WorldNodeProgressPhase>();
		world.entity<WorldScaleProgressPhase>().add(flecs::Phase).depends_on<WorldRegionProgressPhase>();
		world.entity<WorldProgressPhase>().add(flecs::Phase).depends_on<WorldScaleProgressPhase>();

		// Observers
		world.observer<SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each(WorldAddChildQuery);

		world.observer<SpatialWorld3DComponent>()
			.event(flecs::OnRemove)
			.each(WorldRemoveChildQuery);
	}
}