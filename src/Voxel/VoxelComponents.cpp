#include "VoxelComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	VoxelComponents::VoxelComponents(flecs::world& world)
	{
		world.module<VoxelComponents>();

		world.component<VoxelWorldComponent>();

		world.component<VoxelWorldComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::With);
	}
}