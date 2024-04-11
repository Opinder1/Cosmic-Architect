#include "VoxelComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	VoxelComponents::VoxelComponents(flecs::world& world)
	{
		world.module<VoxelComponents>();
	}
}