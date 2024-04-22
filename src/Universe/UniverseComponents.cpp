#include "UniverseComponents.h"
#include "Spatial/SpatialComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// Add a cached query for all of a spatial worlds child nodes for fast access
	void UniverseInitializeSpatialWorld(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
	{
		DEBUG_ASSERT(spatial_world.world == nullptr, "We are trying to assign more than one spatial world to ourself");

		spatial_world.world = new SpatialWorld3D{};
	}

	// Clean up the commands query when destroying a world
	void UniverseUninitializeSpatialWorld(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
	{
		delete spatial_world.world;
	}

	UniverseComponents::UniverseComponents(flecs::world& world)
	{
		world.module<UniverseComponents>();

		world.import<SpatialComponents>();

		world.component<UniverseComponent>();
		world.component<UniverseObjectComponent>();

		world.component<UniverseObjectComponent>().add_second<UniverseComponent>(flecs::OneOf);

		// Observers
		world.observer<UniverseComponent, SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each(UniverseInitializeSpatialWorld);

		world.observer<UniverseComponent, SpatialWorld3DComponent>()
			.event(flecs::OnRemove)
			.each(UniverseUninitializeSpatialWorld);
	}
}