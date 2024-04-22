#include "GalaxyComponents.h"

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

		world.component<GalaxyObjectComponent>().add_second<GalaxyComponent>(flecs::OneOf);
	}
}