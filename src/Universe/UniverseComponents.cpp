#include "UniverseComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<CWorld>();
	}
}