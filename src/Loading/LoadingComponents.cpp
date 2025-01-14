#include "LoadingComponents.h"

#include "EntityLoader.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<EntityLoader>();
		world.component<Identifier>();
		world.component<Saveable>();
		world.component<AutoLoad>();
		world.component<AutoSave>();
		world.component<Database>();
	}
}