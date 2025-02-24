#include "LoadingComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<CIdentifier>();
		world.component<CSaveable>();
		world.component<CDatabase>();
		world.component<ArchiveServer>();
	}
}