#include "LoadingComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<LoadState>();
		world.component<Loadable>();
		world.component<EntityLoader>();

		world.entity<LoadingPhase>();
	}
}