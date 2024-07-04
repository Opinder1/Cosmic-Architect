#include "SpatialComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	template<typename PhaseT, typename DependT>
	void CreateSyncedPhase(flecs::world& world)
	{
		flecs::entity phase = world.entity<PhaseT>()
			.add(flecs::Phase);

		flecs::scoped_world scope = phase.scope();

		flecs::entity sync_phase = scope.entity(DEBUG_ONLY("ThreadSyncPhase"))
			.add(flecs::Phase)
			.depends_on<DependT>();

		scope.system(DEBUG_ONLY("ThreadSyncSystem"))
			.no_readonly()
			.kind(sync_phase)
			.iter([&world](flecs::iter& it)
		{
			DEBUG_THREAD_CHECK_SYNC(&world);
		});

		phase.depends_on(sync_phase);
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
			.add_second<SpatialWorld3DComponent>(flecs::OneOf)
			.add_second<Position3DComponent>(flecs::With);

		world.component<SpatialScale3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialRegion3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialLoader3DComponent>()
			.add_second<SpatialEntity3DComponent>(flecs::With);

		// Phases

		// The phase that all spatial wrold processing happens in
		world.entity<SpatialWorldMultithreadPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		CreateSyncedPhase<WorldRegionWorkerPhase, SpatialWorldMultithreadPhase>(world);
		CreateSyncedPhase<WorldScaleWorkerPhase, WorldRegionWorkerPhase>(world);
		CreateSyncedPhase<WorldWorkerPhase, WorldScaleWorkerPhase>(world);
		CreateSyncedPhase<WorldCreatePhase, WorldWorkerPhase>(world);
		CreateSyncedPhase<WorldLoadPhase, WorldCreatePhase>(world);
		CreateSyncedPhase<WorldUnloadPhase, WorldLoadPhase>(world);
		CreateSyncedPhase<WorldDestroyPhase, WorldUnloadPhase>(world);
		CreateSyncedPhase<WorldEndPhase, WorldDestroyPhase>(world);

		// Observers

		// Add a cached query for all of a spatial worlds child nodes for fast access
		world.observer<SpatialWorld3DComponent>(DEBUG_ONLY("AddSpatialWorldQueries"))
			.event(flecs::OnAdd)
			.each([](flecs::entity world_entity, SpatialWorld3DComponent& spatial_world)
		{
			flecs::scoped_world scope = world_entity.scope(); // Add the queries as children of the entity so they are automatically destructed

			spatial_world.entities_query = scope.query_builder<const SpatialEntity3DComponent>(DEBUG_ONLY("SpatialWorldEntitiesQuery"))
				.term(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.build();

			spatial_world.scale_workers_query = scope.query_builder<const SpatialScale3DWorkerComponent>(DEBUG_ONLY("SpatialWorldScaleWorkersQuery"))
				.term(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.build();

			spatial_world.region_workers_query = scope.query_builder<const SpatialRegion3DWorkerComponent>(DEBUG_ONLY("SpatialWorldRegionWorkersQuery"))
				.term(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.build();

			spatial_world.loaders_query = scope.query_builder<const SpatialLoader3DComponent>(DEBUG_ONLY("SpatialWorldLoadersQuery"))
				.term(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.build();
		});
	}
}