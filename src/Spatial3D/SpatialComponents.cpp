#include "SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<ScaleWorker>();
		world.component<ScaleWorker>();
		world.component<RegionWorker>();
		world.component<Loader>();
		world.component<World>();
		world.component<Entity>();

		world.entity<WorldMultithreadPhase>();
	}
}