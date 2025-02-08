#include "LoadingComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<Identifier>();
		world.component<Saveable>();
		world.component<Database>();
		world.component<ArchiveServer>();
	}
}