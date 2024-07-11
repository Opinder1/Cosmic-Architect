#include "GalaxyComponents.h"

#include "Universe/UniverseComponents.h"

#include "Spatial/SpatialComponents.h"

#include "Physics/PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyComponents::GalaxyComponents(flecs::world& world)
	{
		world.module<GalaxyComponents>("GalaxyComponents");

		world.import<PhysicsComponents>();
		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		world.component<GalaxyComponent>();
		world.component<GalaxyObjectComponent>();
		world.component<StarComponent>();
		world.component<SimulatedGalaxyComponent>();

		world.singleton<GalaxyComponent>()
			.add_second<UniverseComponent>(flecs::OneOf)
			.add_second<Position3DComponent>(flecs::With)
			.add_second<Rotation3DComponent>(flecs::With);

		world.singleton<GalaxyObjectComponent>()
			.add_second<GalaxyComponent>(flecs::OneOf)
			.add_second<SpatialEntity3DComponent>(flecs::With);

		world.singleton<StarComponent>()
			.add_second<GalaxyObjectComponent>(flecs::With);

		world.singleton<SimulatedGalaxyComponent>()
			.add_second<GalaxyComponent>(flecs::With)
			.add_second<SpatialWorld3DComponent>(flecs::With);
	}
}