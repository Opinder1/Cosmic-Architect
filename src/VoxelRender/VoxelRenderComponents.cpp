#include "VoxelRenderComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::voxelrender
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<Node>();
		world.component<World>();
	}
}