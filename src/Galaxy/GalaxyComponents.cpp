#include "GalaxyComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Spatial/SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyComponents::GalaxyComponents(flecs::world& world)
	{
		world.module<GalaxyComponents>("GalaxyComponents");

		world.import<PhysicsComponents>();
		world.import<SpatialComponents>();

		world.component<GalaxyComponent>();
		world.component<GalaxyObjectComponent>();
		world.component<StarComponent>();
		world.component<SimulatedGalaxyComponent>();

		world.component<GalaxyComponent>();

		world.component<GalaxyObjectComponent>()
			.add_second<GalaxyComponent>(flecs::OneOf);

		world.component<StarComponent>();

		world.component<SimulatedGalaxyComponent>();
	}
}