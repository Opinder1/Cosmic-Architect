#include "SpatialComponents.h"

namespace voxel_game::spatial3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<CLoader>();
		world.component<CWorld>();

		world.component<PNodeCreate>();
		world.component<PNodeLoad>();
		world.component<PNodeUnload>();
		world.component<PNodeDestroy>();

		world.component<EWorldCreate>();
		world.component<EWorldDestroy>();
	}
}