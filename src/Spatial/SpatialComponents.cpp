#include "SpatialComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// Add a cached query for all of a spatial worlds child nodes for fast access
	void WorldAddChildQuery(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		flecs::scoped_world scope = entity.world().scope(entity); // Add the queries as children of the entity so they are automatically destructed

		// Use read() as its required for queries run inside systems
		spatial_world.loaders_query = scope.query_builder<const SpatialLoader3DComponent>("SpatialWorldLoaderQuery")
			.term(flecs::ChildOf, entity).read()
			.build();
	}

	SpatialComponents::SpatialComponents(flecs::world& world)
	{
		world.module<SpatialComponents>();

		world.import<PhysicsComponents>();

		// Components
		world.component<SpatialEntity3DComponent>();
		world.component<SpatialWorld3DComponent>();
		world.component<SpatialScale3DWorkerComponent>();
		world.component<SpatialScale3DWorkerComponent>();
		world.component<SpatialRegion3DWorkerComponent>();
		world.component<SpatialNode3DWorkerComponent>();
		world.component<SpatialLoader3DComponent>();

		// Relationships
		world.component<SpatialEntity3DComponent>()
			.add_second<Position3DComponent>(flecs::With)
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialScale3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialRegion3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialNode3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialLoader3DComponent>()
			.add_second<Position3DComponent>(flecs::With)
			.add_second<SpatialEntity3DComponent>(flecs::With);

		// Phases
		world.entity<WorldLoaderWorkerPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		world.entity<WorldNodeWorkerPhase>()
			.add(flecs::Phase)
			.depends_on<WorldLoaderWorkerPhase>();

		world.entity<WorldRegionWorkerPhase>()
			.add(flecs::Phase)
			.depends_on<WorldNodeWorkerPhase>();

		world.entity<WorldScaleWorkerPhase>()
			.add(flecs::Phase)
			.depends_on<WorldRegionWorkerPhase>();

		world.entity<WorldWorkerPhase>()
			.add(flecs::Phase)
			.depends_on<WorldScaleWorkerPhase>();

		// Observers
		world.observer<SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each(WorldAddChildQuery);
	}
}