#include "VoxelComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::voxel
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<VoxelWorldComponent>();
	}
}