#include "GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyComponents::GalaxyComponents(flecs::world& world)
	{
		world.module<GalaxyComponents>();

		world.component<GalaxyComponent>();
		world.component<GalaxyObjectComponent>();
		world.component<StarComponent>();
		world.component<SimulatedGalaxyComponent>();

		world.component<GalaxyComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::With);

		world.component<GalaxyObjectComponent>()
			.add_second<GalaxyComponent>(flecs::OneOf)
			.add_second<SpatialEntity3DComponent>(flecs::With);

		world.component<StarComponent>()
			.add_second<GalaxyObjectComponent>(flecs::With);

		world.component<SimulatedGalaxyComponent>()
			.add_second<GalaxyComponent>(flecs::With);
	}
}