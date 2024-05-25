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
		spatial_world.loaders_query = scope.query_builder<const SpatialLoader3DComponent>(DEBUG_ONLY("SpatialWorldLoaderQuery"))
			.term(flecs::ChildOf, entity).read()
			.build();
	}

	template<typename PhaseT, typename DependT>
	void CreateSyncedPhase(flecs::world& world)
	{
		world.entity<PhaseT>()
			.add(flecs::Phase)
			.depends_on<DependT>();

		world.system()
			.no_readonly()
			.kind<PhaseT>()
			.iter([](flecs::iter&) {});

#if DEBUG
		world.system<const SpatialScale3DWorkerComponent>("ProfileSlowSystem")
			.multi_threaded()
			.kind<PhaseT>()
			.each([](const SpatialScale3DWorkerComponent&)
		{
			for (volatile size_t i = 0; i < 1000 * 1000; i++) {}
		});
#endif
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
		world.component<SpatialLoader3DComponent>();

		// Relationships
		world.component<SpatialEntity3DComponent>()
			.add_second<Position3DComponent>(flecs::With)
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialScale3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialRegion3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialLoader3DComponent>()
			.add_second<Position3DComponent>(flecs::With)
			.add_second<SpatialEntity3DComponent>(flecs::With);

		// The phase that all spatial wrold processing happens in
		world.entity<SpatialWorldMultithreadPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		CreateSyncedPhase<WorldLoaderWorkerPhase, SpatialWorldMultithreadPhase>(world);
		CreateSyncedPhase<WorldRegionWorkerPhase, WorldLoaderWorkerPhase>(world);
		CreateSyncedPhase<WorldScaleWorkerPhase, WorldRegionWorkerPhase>(world);
		CreateSyncedPhase<WorldWorkerPhase, WorldScaleWorkerPhase>(world);
		CreateSyncedPhase<WorldCreatePhase, WorldWorkerPhase>(world);
		CreateSyncedPhase<WorldLoadPhase, WorldCreatePhase>(world);
		CreateSyncedPhase<WorldUnloadPhase, WorldLoadPhase>(world);
		CreateSyncedPhase<WorldDestroyPhase, WorldUnloadPhase>(world);
		CreateSyncedPhase<WorldMultiworldPhase, WorldDestroyPhase>(world);

		// Observers
		world.observer<SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each(WorldAddChildQuery);
	}
}