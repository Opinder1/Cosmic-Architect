#include "GalaxyComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<Galaxy>();
		world.component<Star>();
	}
}