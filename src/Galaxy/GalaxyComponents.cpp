#include "GalaxyComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyComponents::GalaxyComponents(flecs::world& world)
	{
		world.module<GalaxyComponents>();

		world.component<GalaxyComponent>();
		world.component<StarComponent>();
	}
}