#include "UniverseComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseComponents::UniverseComponents(flecs::world& world)
	{
		world.module<UniverseComponents>();

		world.component<UniverseComponent>();
		world.component<UniverseObjectComponent>();

		world.component<UniverseObjectComponent>().add_second<UniverseComponent>(flecs::OneOf);
	}
}