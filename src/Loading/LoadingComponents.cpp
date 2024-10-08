#include "LoadingComponents.h"

#include "EntityLoader.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<EntityLoader>();
	}
}